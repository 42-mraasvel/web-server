#include "CgiHandler.hpp"
#include "settings.hpp"
#include "utility/utility.hpp"
#include "utility/macros.hpp"
#include "utility/status_codes.hpp"
#include <libgen.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <cstdlib>
#include <cstring>
#include <sys/wait.h>

// Configuration syntax: CGI .py /usr/bin/python3

CgiHandler::CgiHandler()
: _status(CgiHandler::INACTIVE), _status_code(0), _sender(NULL), _reader(NULL), _cgi_pid(-1) {}

CgiHandler::~CgiHandler()
{
	destroyFds();
	cleanCgi();
}

bool CgiHandler::isCgi(std::string const & request_target, CgiVectorType const & cgi)
{
	for (CgiVectorType::const_iterator it = cgi.begin(); it != cgi.end(); ++it)
	{
		if (WebservUtility::stringEndsWith(request_target, it->first))
		{
			return true;
		}
	}
	return false;
}

bool CgiHandler::isCgi(Request const & request)
{
	if (request.config_info.result == ConfigInfo::NOT_FOUND)
	{
		return false;
	}
	return isCgi(request.config_info.resolved_target, request.config_info.resolved_location->_cgi);
}

/*
1. Preparation: ScriptLocation, MetaVariables

	- ScriptLocation: based off Configuration options
	- MetaVariables

2. Open pipes to connect STDIN and STDOUT, create new instances
3. Fork and execute the script, store the PID internally
4. Close unused pipe ends
*/
int CgiHandler::executeRequest(FdTable& fd_table, Request& request)
{
	printf(YELLOW_BOLD "-- Executing CGI --" RESET_COLOR "\n");

	/* 1. Preparation */
	_status = CgiHandler::INCOMPLETE;
	setInfo(request.config_info);
	generateMetaVariables(request);

	if (!scriptCanBeExecuted())
	{
		fprintf(stderr, "%sCgiHandler%s: Cgi Script: %s: cannot be executed\n",
			RED_BOLD, RESET_COLOR, _script.c_str());
		finishCgi(ERROR, StatusCode::BAD_GATEWAY);
		return ERR;
	}

	/* 2. Open pipes, create FdClasses */
	int fds[2] = {-1, -1};
	if (initializeCgiConnection(fds, fd_table, request) == ERR)
	{
		finishCgi(ERROR, StatusCode::INTERNAL_SERVER_ERROR);
		return ERR;
	}

	/* 3. Fork */
	print();
	if (forkCgi(fds, fd_table) == ERR)
	{
		finishCgi(ERROR, StatusCode::INTERNAL_SERVER_ERROR);
		WebservUtility::closePipe(fds);
		return ERR;
	}

	WebservUtility::closePipe(fds);
	_meta_variables.clear();
	_timer.reset();
	return OK;
}

void CgiHandler::setInfo(ConfigInfo const & info)
{
	_root_dir = info.resolved_location->_root;
	_target = info.resolved_file_path;
	resolveCgiScript(info.resolved_target, info.resolved_location->_cgi);
}

void CgiHandler::resolveCgiScript(std::string const target, CgiVectorType const & cgi)
{
	std::size_t index = 0;
	for (CgiVectorType::const_iterator it = cgi.begin(); it != cgi.end(); ++it)
	{
		if (WebservUtility::stringEndsWith(target, it->first))
		{
			_script = it->second;
			break;
		}
	}
}

bool CgiHandler::scriptCanBeExecuted()
{
	return access(_script.c_str(), X_OK) == 0;
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
void CgiHandler::generateMetaVariables(const Request& request)
{
	metaVariableContent(request);
	_meta_variables.push_back(MetaVariableType("GATEWAY_INTERFACE", "CGI/1.1"));
	metaVariablePathInfo(request);
	_meta_variables.push_back(MetaVariableType("QUERY_STRING", request.query.c_str()));
	_meta_variables.push_back(MetaVariableType("REMOTE_ADDR", request.address.first));
	_meta_variables.push_back(MetaVariableType("REQUEST_METHOD", request.getMethodString()));
	_meta_variables.push_back(MetaVariableType("SERVER_NAME", request.interface_addr.first));
	_meta_variables.push_back(MetaVariableType("SERVER_PORT", WebservUtility::itoa(request.interface_addr.second)));
	_meta_variables.push_back(MetaVariableType("SERVER_PROTOCOL", "HTTP/1.1"));
	_meta_variables.push_back(MetaVariableType("SERVER_SOFTWARE", "Plebserv Remastered"));
	metaVariableHeader(request);
}

void CgiHandler::metaVariableContent(const Request& request)
{
	if (request.message_body.size() == 0)
	{
		return;
	}

	_meta_variables.push_back(MetaVariableType("CONTENT_LENGTH", WebservUtility::itoa(request.message_body.size())));
	HeaderField::const_pair_type p = request.header_fields.get("content-type");
	if (p.second)
	{
		_meta_variables.push_back(MetaVariableType("CONTENT_TYPE", p.first->second));
	}
}

void CgiHandler::metaVariablePathInfo(const Request& request)
{
	//TODO: test with the executable on intra, since the subj states full path should be given
	_meta_variables.push_back(MetaVariableType("PATH_INFO", request.config_info.resolved_target));
	// _meta_variables.push_back(MetaVariableType("PATH_INFO", _root_dir + request.config_info.resolved_target));
	_meta_variables.push_back(MetaVariableType("PATH_TRANSLATED", _root_dir + request.config_info.resolved_target));
}

void CgiHandler::metaVariableHeader(const Request& request)
{
	for (HeaderField::const_iterator it = request.header_fields.begin();
		it != request.header_fields.end(); ++it)
	{
		_meta_variables.push_back(convertFieldToMeta(it->first, it->second));
	}
}

CgiHandler::MetaVariableType CgiHandler::convertFieldToMeta(
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

/* Setting up FDs, pipes and connections to the CGI */

/*
Initializes the CgiReader, CgiSender classes
Stores the FDs used inside of the CGI in rfds
*/
int CgiHandler::initializeCgiConnection(int* cgi_fds, FdTable& fd_table, Request& r)
{
	if (initializeCgiReader(cgi_fds, fd_table) == ERR)
	{
		return ERR;
	}

	if (initializeCgiSender(cgi_fds, fd_table, r) == ERR)
	{
		if (close(cgi_fds[1]) == ERR)
		{
			syscallError(_FUNC_ERR("close"));
		}
		return ERR;
	}
	return OK;
}

int CgiHandler::initializeCgiReader(int* cgi_fds, FdTable& fd_table)
{
	int fds[2];

	if (pipe(fds) == ERR)
	{
		return syscallError(_FUNC_ERR("pipe"));
	}

	// Reader needs the READ end of the pipe, so it gives the WRITE end to the CGI
	cgi_fds[1] = fds[1];
	// Instantiate the CgiReader with the READ end of the pipe and add it to the FdTable.
	if (WebservUtility::makeNonBlocking(fds[0]) == ERR)
	{
		WebservUtility::closePipe(fds);
		return syscallError(_FUNC_ERR("fcntl"));
	}



	try
	{
		_reader = SmartPointer<CgiReader>(new CgiReader(fds[0], &_timer));
		fd_table.insertFd(SmartPointer<AFdInfo>(_reader));
	}
	catch (...)
	{
		WebservUtility::closePipe(fds);
		_reader = NULL;
		throw;
	}
	return OK;
}

int CgiHandler::initializeCgiSender(int* cgi_fds, FdTable& fd_table, Request& r)
{
	int fds[2];

	if (pipe(fds) == ERR)
	{
		return syscallError(_FUNC_ERR("pipe"));
	}

	// Sender needs the WRITE end of the pipe, so it gives the READ end to the CGI
	cgi_fds[0] = fds[0];

	// Instantiate the CgiSender with the WRITE end of the pipe and add it to the FdTable
	if (WebservUtility::makeNonBlocking(fds[1]) == ERR)
	{
		WebservUtility::closePipe(fds);
		return syscallError(_FUNC_ERR("fcntl"));
	}

	/* Exception safe code */
	try
	{
		_sender = SmartPointer<CgiSender>(new CgiSender(fds[1], &r, &_timer));
		fd_table.insertFd(SmartPointer<AFdInfo>(_sender));
	}
	catch (...)
	{
		WebservUtility::closePipe(fds);
		WebservUtility::closePipe(cgi_fds);
		_sender = NULL;
		throw;
	}
	return OK;
}

/* Forking, Execve'ing */

int CgiHandler::forkCgi(int* cgi_fds, FdTable& fd_table)
{
	_cgi_pid = fork();
	if (_cgi_pid == ERR)
	{
		return syscallError(_FUNC_ERR("fork"));
	}
	else if (_cgi_pid == 0)
	{
		// We can exit because it's the child process
		if (prepareCgi(cgi_fds, fd_table) == ERR)
		{
			exit(EXIT_FAILURE);
		}
		exit(executeChildProcess());
	}
	return OK;
}

/*
1. Execve _target

Executable name: defined in the CGI
First argument: executable basename
Second argument: _target
*/
int CgiHandler::executeChildProcess() const
{
	char* args[3];

	if (prepareArguments(args) == ERR)
	{
		return ERR;
	}
	// TODO: chdir into the target resource or root directory?
	if (chdir(_root_dir.c_str()) == ERR)
	{
		return syscallError("chdir");
	}

	execve(_script.c_str(), args, WebservUtility::getEnvp());
	// Execve only returns on ERROR
	return syscallError(_FUNC_ERR("execve"));
}

int CgiHandler::prepareArguments(char *args[3]) const
{
	/*
	First argument: executable basename
	Second argument: _target
	*/
	args[0] = strdup(WebservUtility::ft_basename(_script.c_str()));
	if (args[0] == NULL)
	{
		return ERR;
	}
	args[1] = realpath(_target.c_str(), NULL);
	if (args[1] == NULL)
	{
		free(args[0]);
		return ERR;
	}
	args[2] = NULL;
	return OK;
}

/*
1. Close all file descriptors in FdTable that are not needed for the child process
2. Initialize environment
3. Initialize stdin, stdout through dup2
*/
int CgiHandler::prepareCgi(int* cgi_fds, FdTable& fd_table) const
{
	if (closeAll(fd_table) == ERR)
	{
		return ERR;
	}

	if (setEnvironment() == ERR)
	{
		return ERR;
	}

	if (setRedirection(cgi_fds) == ERR)
	{
		return ERR;
	}

	return OK;
}

int CgiHandler::closeAll(FdTable& fd_table) const
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

int CgiHandler::setEnvironment() const
{
	for (MetaVariableContainerType::const_iterator it = _meta_variables.begin();
		it != _meta_variables.end(); ++it)
	{
		if (setenv(it->first.c_str(), it->second.c_str(), true) == ERR)
		{
			return syscallError(_FUNC_ERR("setenv"));
		}
	}

	return OK;
}

int CgiHandler::setRedirection(int* cgi_fds) const
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

bool CgiHandler::isExecutionError() const
{
	return (_reader && _reader->getFlag() == AFdInfo::ERROR)
		|| (_sender && _sender->getFlag() == AFdInfo::ERROR);
}

int CgiHandler::getErrorCode() const
{
	if (_reader && _reader->getFlag() == AFdInfo::ERROR)
	{
		return _reader->getStatusCode();
	}
	
	return _sender->getStatusCode();
}

/*
This function should only be called once
*/
void CgiHandler::setSpecificHeaderField(HeaderField & header_field, bool content_type_fixed)
{
	if (_reader && _header.size() == 0)
	{
		_header.swap(_reader->getHeader());
	}
	for (HeaderField::const_iterator it = _header.begin(); it != _header.end(); ++it)
	{
		// Don't add Content-Length if TE is present
		if (WebservUtility::caseInsensitiveEqual(it->first, "content-length")
			&& header_field.contains("Transfer-Encoding"))
		{
			continue;
		}

		//TODO: check if should be removed
		if (header_field.contains(it->first) && it->second != header_field[it->first])
		{
			fprintf(stderr, "  %sWARNING%s: %s:%d [%s]: Overwriting Field: %s: [%s] with [%s]\n",
				RED_BOLD, RESET_COLOR,
				__FILE__, __LINE__, __FUNCTION__, it->first.c_str(),
				header_field[it->first].c_str(), it->second.c_str());
		}
		if (!skippedHeaderField(it->first))
		{
			header_field[it->first] = it->second;
		}
	}
	_header.clear();
}

bool CgiHandler::skippedHeaderField(std::string const & key) const
{
	static const std::string skipped_fields[] = {
		"status"
	};

	for (std::size_t i = 0; i < sizeof(skipped_fields) / sizeof(skipped_fields[0]); ++i)
	{
		if (WebservUtility::caseInsensitiveEqual(skipped_fields[i], key))
		{
			return true;
		}
	}
	return false;
}

/*
Function's purpose:
	- Check if ERROR occured, CGI child process status (still alive etc)
		IF exited: wait
		IF exited AND sender is INCOMPLETE: BAD_GATEWAY
	- Check if either Sender or Reader is complete: remove from table if so
	- Set completion status if both are complete

TODO: ERROR handling
	- CGI program times out (inf loop, takes too long to produce content)
	- Sender has not completely finished writing it's content (POLLERR + closeEvent())
		: Example: reader has finished reading a valid response, but the sender is still not done and the CGI exited
*/
// TODO: aileen: added response_body to directly copy content to
void CgiHandler::update(std::string & response_body)
{
	//return if already finished communicating with CGI
	if (isComplete() || isError())
	{
		return;
	}

	if (isExecutionError())
	{
		finishCgi(CgiHandler::ERROR, getErrorCode());
		return;
	}

	if (_reader)
	{
		evaluateReader(response_body);
	}

	if (_sender && _sender->getFlag() == AFdInfo::COMPLETE)
	{
		_sender->setToErase();
		_sender = NULL;
	}


	if (isComplete())
	{
		finishCgi(CgiHandler::COMPLETE, checkStatusField());
	}
	else if (_timer.elapsed() >= TIMEOUT)
	{
		printf("%sCgiHandler%s: TIMEOUT\n", RED_BOLD, RESET_COLOR); 
		finishCgi(CgiHandler::ERROR, StatusCode::GATEWAY_TIMEOUT);
	}
}

void CgiHandler::evaluateReader(std::string & response_body)
{
	if (_reader->getBody().size() > 0)
	{
		if (response_body.size() == 0)
		{
			response_body.swap(_reader->getBody());
		}
		else
		{
			response_body.append(_reader->getBody());
			_reader->getBody().clear();
		}
	}
	if (_reader->getFlag() == AFdInfo::COMPLETE)
	{
		_reader->setToErase();
		_reader = NULL;
	}
}

void CgiHandler::exceptionEvent()
{
	clear();
	finishCgi(CgiHandler::ERROR, StatusCode::INTERNAL_SERVER_ERROR);
	fprintf(stderr, "%sEXCEPTION%s: CgiHandler\n", RED_BOLD, RESET_COLOR);
}

void CgiHandler::clear()
{
	_root_dir.clear();;
	_script.clear();
	_target.clear();
	_meta_variables.clear();;
	_header.clear();
}

int CgiHandler::checkStatusField() const
{
	HeaderField::const_pair_type status = _header.get("Status");
	if (status.second)
	{
		printf("Cgi returned StatusCode: %s\n", status.first->second.c_str());
		return WebservUtility::strtol(status.first->second);
	}
	return StatusCode::STATUS_OK;
}

int CgiHandler::cleanCgi()
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
	// TODO: Check exit status, crash, signal etc
	_cgi_pid = -1;
	return OK;
}

int CgiHandler::killCgi(int* status)
{
	printf(BLUE_BOLD "WaitEvent CGI:" RESET_COLOR " PID(%d)\n", _cgi_pid);
	pid_t result = waitpid(_cgi_pid, status, WNOHANG);
	if (result == ERR)
	{
		return syscallError(_FUNC_ERR("waitpid"));
	}
	else if (result == 0)
	{
		printf("  CGI is still alive and has to be killed: [%d]\n", _cgi_pid);
		if (kill(_cgi_pid, SIGKILL) == ERR)
		{
			return syscallError(_FUNC_ERR("kill"));
		}

		printf("  Waiting for CGI after killing\n");
		if (waitpid(_cgi_pid, status, 0) == ERR)
		{
			return syscallError(_FUNC_ERR("waitpid"));
		}
	}
	return OK;
}

/* Utilities */

bool CgiHandler::isComplete() const
{
	return _status != CgiHandler::INACTIVE && _sender == NULL && _reader == NULL;
}

bool CgiHandler::isError() const
{
	return _status == CgiHandler::ERROR;
}

bool CgiHandler::isReadyToWrite() const
{
	return isComplete() || isError();
}

void CgiHandler::finishCgi(Status status, int code)
{
	cleanCgi();
	destroyFds();
	_status = status;
	_status_code = code;
}

/* Clean up, destroying */

void CgiHandler::destroyFds()
{
	if (_sender)
	{
		_sender->setToErase();
		_sender = NULL;
	}

	if (_reader)
	{
		_reader->setToErase();
		_reader = NULL;
	}
}

/* Getters */

int CgiHandler::getStatusCode() const
{
	return _status_code;
}

void CgiHandler::print() const {

	printf("TARGET: %s\n", _target.c_str());
	printf("SCRIPT: %s\n", _script.c_str());
	for (MetaVariableContainerType::const_iterator it = _meta_variables.begin();
		it != _meta_variables.end(); ++it)
	{
		printf("%s: %s\n", it->first.c_str(), it->second.c_str());
	}
}
