#include "CgiExecutor.hpp"
#include "parser/Request.hpp"
#include "settings.hpp"
#include "utility/status_codes.hpp"
#include "utility/utility.hpp"
#include "fd/FdTable.hpp"
#include "fd/AFdInfo.hpp"
#include <sys/wait.h>
#include <csignal>
#include <cstdlib>
#include <cstring>
#include <unistd.h>

CgiExecutor::CgiExecutor()
: _status_code(0), _cgi_pid(-1) {}

CgiExecutor::~CgiExecutor()
{
	clearCgi();
}

int CgiExecutor::execute(FdTable & fd_table, Request & request, int* cgi_fds)
{
	std::string script = resolveCgiScript(request.config_info.resolved_target, request.config_info.resolved_location->_cgi);

	if (!canBeExecuted(script))
	{
		PRINT_WARNING << "CgiExecutor: Cgi Script: " << script << ": cannot be executed" << std::endl;
		setStatus(StatusCode::BAD_GATEWAY);
		return ERR;
	}

	MetaVariableContainerType meta_variables = generateMetaVariables(request);

	print(meta_variables, script, request.config_info);
	if (forkCgi(cgi_fds, fd_table, script, request.config_info, meta_variables) == ERR)
	{
		setStatus(StatusCode::INTERNAL_SERVER_ERROR);
		return ERR;
	}
	return OK;
}

std::string CgiExecutor::resolveCgiScript(std::string const target, CgiVectorType const & cgi)
{
	std::size_t index = 0;
	for (CgiVectorType::const_iterator it = cgi.begin(); it != cgi.end(); ++it)
	{
		if (WebservUtility::stringEndsWith(target, it->first))
		{
			return it->second;
		}
	}
	return "";
}

/* Setting up the meta-variables (environment) */

/*
	CONTENT_LENGTH		From Request Body
	CONTENT_TYPE		From Request Header
	GATEWAY_INTERFACE	Defaults to `CGI/1.1`
	PATH_INFO			(/index.php)
	PATH_TRANSLATED		Request-Target translated to a local URI (/var/www/html/index.php)
	QUERY_STRING		Query part of the request-target (/index.php?1=2&x=y) => 1=2&x=y
	REMOTE_ADDR			IPv4 address of the connected client
	REQUEST_METHOD		GET | POST | DELETE
	SERVER_NAME			IPv4 address the client used to connect to the server
	SERVER_PORT			Port client used to connect to the server
	SERVER_PROTOCOL		`HTTP/1.1`
	SERVER_SOFTWARE		Name of the server

	HTTP_* fields		All other header-fields given by the Request
*/
CgiExecutor::MetaVariableContainerType CgiExecutor::generateMetaVariables(const Request& request)
{
	MetaVariableContainerType meta_variables;

	metaVariableContent(request, meta_variables);
	meta_variables.push_back(MetaVariableType("GATEWAY_INTERFACE", "CGI/1.1"));
	metaVariablePathInfo(request, meta_variables);
	meta_variables.push_back(MetaVariableType("QUERY_STRING", request.query.c_str()));
	meta_variables.push_back(MetaVariableType("REMOTE_ADDR", request.address.first));
	meta_variables.push_back(MetaVariableType("REQUEST_METHOD", request.getMethodString()));
	meta_variables.push_back(MetaVariableType("SERVER_NAME", request.interface_addr.first));
	meta_variables.push_back(MetaVariableType("SERVER_PORT", WebservUtility::itoa(request.interface_addr.second)));
	meta_variables.push_back(MetaVariableType("SERVER_PROTOCOL", "HTTP/1.1"));
	meta_variables.push_back(MetaVariableType("SERVER_SOFTWARE", "Plebserv Remastered"));
	metaVariableHeader(request, meta_variables);
	return meta_variables;
}

void CgiExecutor::metaVariableContent(const Request& request, MetaVariableContainerType & meta_variables)
{
	if (request.message_body.size() == 0)
	{
		return;
	}

	meta_variables.push_back(MetaVariableType("CONTENT_LENGTH", WebservUtility::itoa(request.message_body.size())));
	HeaderField::const_pair_type p = request.header_fields.get("content-type");
	if (p.second)
	{
		meta_variables.push_back(MetaVariableType("CONTENT_TYPE", p.first->second));
	}
}

void CgiExecutor::metaVariablePathInfo(const Request& request, MetaVariableContainerType & meta_variables)
{
	meta_variables.push_back(MetaVariableType("PATH_INFO", request.config_info.resolved_target));
	meta_variables.push_back(MetaVariableType("PATH_TRANSLATED", request.config_info.resolved_location->_root + request.config_info.resolved_target));
}

void CgiExecutor::metaVariableHeader(const Request& request, MetaVariableContainerType & meta_variables)
{
	for (HeaderField::const_iterator it = request.header_fields.begin();
		it != request.header_fields.end(); ++it)
	{
		meta_variables.push_back(convertFieldToMeta(it->first, it->second));
	}
}

CgiExecutor::MetaVariableType CgiExecutor::convertFieldToMeta(
		const std::string& key, const std::string& value) const
{
	std::string result;
	for (std::size_t i = 0; i < key.size(); ++i)
	{
		if (key[i] == '-') {
			result.push_back('_');
		} else {
			result.push_back(toupper(key[i]));
		}
	}
	return MetaVariableType("HTTP_" + result, value);
}


bool CgiExecutor::canBeExecuted(std::string const & script)
{
	return access(script.c_str(), X_OK) == 0;
}

/*
Execution
*/

int CgiExecutor::forkCgi(int* cgi_fds, FdTable& fd_table, std::string const & script,
					ConfigInfo const & info, MetaVariableContainerType const & meta_variables)
{
	_cgi_pid = fork();
	if (_cgi_pid == ERR)
	{
		return syscallError(_FUNC_ERR("fork"));
	}
	else if (_cgi_pid == 0)
	{
		// We can exit because it's the child process
		if (prepareCgi(cgi_fds, fd_table, meta_variables) == ERR)
		{
			exit(EXIT_FAILURE);
		}
		exit(executeChildProcess(script, info));
	}
	return OK;
}

/*
1. Close all file descriptors in FdTable that are not needed for the child process
2. Initialize environment
3. Initialize stdin, stdout through dup2
*/
int CgiExecutor::prepareCgi(int* cgi_fds, FdTable& fd_table, MetaVariableContainerType const & meta_variables) const
{
	if (closeAll(fd_table) == ERR)
	{
		return ERR;
	}

	if (setEnvironment(meta_variables) == ERR)
	{
		return ERR;
	}

	if (setRedirection(cgi_fds) == ERR)
	{
		return ERR;
	}

	return OK;
}

int CgiExecutor::closeAll(FdTable& fd_table) const
{
	for (std::size_t i = 0; i < fd_table.size(); ++i)
	{
		if (close(fd_table[i].second->getFd()) == ERR)
		{
			syscallError(_FUNC_ERR("close"));
		}
	}
	return OK;
}

int CgiExecutor::setEnvironment(MetaVariableContainerType const & meta_variables) const
{
	for (MetaVariableContainerType::const_iterator it = meta_variables.begin();
		it != meta_variables.end(); ++it)
	{
		if (setenv(it->first.c_str(), it->second.c_str(), true) == ERR)
		{
			return syscallError(_FUNC_ERR("setenv"));
		}
	}

	return OK;
}

int CgiExecutor::setRedirection(int* cgi_fds) const
{
	if (dup2(cgi_fds[0], STDIN_FILENO) == ERR)
	{
		return syscallError(_FUNC_ERR("dup2"));
	}
	if (dup2(cgi_fds[1], STDOUT_FILENO) == ERR)
	{
		return syscallError(_FUNC_ERR("dup2"));
	}

	WebservUtility::closePipe(cgi_fds);
	return OK;
}

/*
1. Execve _target

Executable name: defined in the CGI
First argument: executable basename
Second argument: _target
*/

int CgiExecutor::executeChildProcess(std::string const & script, ConfigInfo const & info) const
{
	char* args[3];

	if (prepareArguments(args, script, info) == ERR)
	{
		return ERR;
	}
	//TODO: DISCUSS: chdir into the target resource or root directory?
	if (chdir(info.resolved_location->_root.c_str()) == ERR)
	{
		return syscallError("chdir");
	}

	execve(script.c_str(), args, WebservUtility::getEnvp());
	// Execve only returns on ERROR
	return syscallError(_FUNC_ERR("execve"));
}

int CgiExecutor::prepareArguments(char *args[3], std::string const & script, ConfigInfo const & info) const
{
	/*
	First argument: executable basename
	Second argument: _target
	*/
	args[0] = strdup(WebservUtility::ft_basename(script.c_str()));
	if (args[0] == NULL)
	{
		return ERR;
	}
	args[1] = realpath(info.resolved_file_path.c_str(), NULL);
	if (args[1] == NULL)
	{
		free(args[0]);
		return ERR;
	}
	args[2] = NULL;
	return OK;
}

/*
Cleaning up
*/

int CgiExecutor::clearCgi()
{
	if (_cgi_pid == -1)
	{
		return OK;
	}

	int status;
	if (killCgi(&status) == ERR)
	{
		return ERR;
	}
	_cgi_pid = -1;
	return OK;
}

int CgiExecutor::killCgi(int* status)
{
	PRINT_INFO << BLUE_BOLD "CgiExecutor: " RESET_COLOR "cleaning child PID: " << _cgi_pid << std::endl;
	pid_t result = waitpid(_cgi_pid, status, WNOHANG);
	if (result == ERR)
	{
		return syscallError(_FUNC_ERR("waitpid"));
	}
	else if (result == 0)
	{
		if (kill(_cgi_pid, SIGKILL) == ERR)
		{
			return syscallError(_FUNC_ERR("kill"));
		}

		if (waitpid(_cgi_pid, status, 0) == ERR)
		{
			return syscallError(_FUNC_ERR("waitpid"));
		}
	}
	return OK;
}

/*
Simple setters
*/
int CgiExecutor::getStatusCode() const
{
	return _status_code;
}

void CgiExecutor::setStatus(int status_code)
{
	_status_code = status_code;
}

void CgiExecutor::clear()
{
	clearCgi();
}

void CgiExecutor::print(MetaVariableContainerType const & meta_variables, std::string const & script, ConfigInfo const & info) const
{
	PRINT_DEBUG << "Target: " << info.resolved_file_path << std::endl;
	PRINT_DEBUG << "Script: " << script << std::endl;
	for (MetaVariableContainerType::const_iterator it = meta_variables.begin();
		it != meta_variables.end(); ++it)
	{
		PRINT_DEBUG << it->first << ": " << it->second << std::endl;
	}
}
