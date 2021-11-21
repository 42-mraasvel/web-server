#include "CgiHandler.hpp"
#include "settings.hpp"
#include "utility/utility.hpp"
#include "utility/macros.hpp"
#include "CgiSender.hpp"
#include "CgiReader.hpp"
#include "utility/status_codes.hpp"
#include <libgen.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <cstdlib>
#include <cstring>
#include <sys/wait.h>

// Change the root directories if you want to test CGI
#ifdef __linux__
	#define WEBSERV_DIR "/home/mraasvel/work/codam/webserv-pyxis/"
	#define BIN_DIR "/usr/bin/"
#else /* __linux__ */
	#define WEBSERV_DIR "/Users/mraasvel/work/codam/webserv-pyxis/"
	#define BIN_DIR "/Users/mraasvel/.brew/bin/"
#endif /* __linux__ */

#define SCRIPT_PATH_BASE "python3"
#define CGI_EXTENSION ".py"
#define SERVER_ROOT WEBSERV_DIR "page_sample"
#define SCRIPT_PATH BIN_DIR SCRIPT_PATH_BASE

// Configuration syntax: CGI .py /usr/bin/python3

CgiHandler::CgiHandler()
: _status(CgiHandler::INACTIVE), _sender(NULL), _reader(NULL), _cgi_pid(-1) {}

CgiHandler::~CgiHandler()
{
	if (_status != CgiHandler::INACTIVE)
	{
		destroyFds();
		cleanCgi();
	}
}

static std::size_t findCgiComponent(std::string const & request_target, std::string const & extension)
{
	std::size_t index = 0;
	while (true)
	{
		std::size_t end = request_target.find("/", index + 1);
		if (WebservUtility::stringEndsWith(request_target, extension, index, end))
		{
			return index;
		}
		else if (end == std::string::npos)
		{
			break;
		}
		index = end;
	}
	return std::string::npos;
}

bool CgiHandler::isCgi(std::string const & request_target, CgiVectorType const & cgi)
{
	for (CgiVectorType::const_iterator it = cgi.begin(); it != cgi.end(); ++it)
	{
		if (findCgiComponent(request_target, it->first) != std::string::npos)
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

void CgiHandler::splitRequestTarget(std::string const & request_target, CgiVectorType const & cgi)
{
	std::size_t index = 0;
	for (CgiVectorType::const_iterator it = cgi.begin(); it != cgi.end(); ++it)
	{
		index = findCgiComponent(request_target, it->first);
		if (index != std::string::npos)
		{
			_script = it->second;
			break;
		}
	}
	std::size_t end = request_target.find("/", index + 1);
	_target = request_target.substr(0, end);
	if (end != std::string::npos) {
		_meta_variables.push_back(MetaVariableType("PATH_INFO", request_target.substr(end)));
	} else {
		_meta_variables.push_back(MetaVariableType("PATH_INFO", ""));
	}
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
	splitRequestTarget(request.config_info.resolved_target, request.config_info.resolved_location->_cgi);
	generateMetaVariables(request);
	_target = request.config_info.resolved_location->_root + _target;

	if (!scriptCanBeExecuted())
	{
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
	if (forkCgi(fds, fd_table) == ERR)
	{
		finishCgi(ERROR, StatusCode::INTERNAL_SERVER_ERROR);
		WebservUtility::closePipe(fds);
		return ERR;
	}

	/* 4. Clean up CGI: should be done in 'FinishCgi' maybe? */
	WebservUtility::closePipe(fds);
	_meta_variables.clear();
	return OK;
}

bool CgiHandler::scriptCanBeExecuted()
{
	return access(_script.c_str(), X_OK) == 0;
}

/* Setting up the meta-variables (environment) */

/*
Already known: PATH_INFO

To Generate:

	Content-Length, Content-Type (message-body related)
	[OPTIONAL] PATH_TRANSLATED: Extract from PATH_INFO
	[OPTIONAL] REMOTE_HOST: Hostname of client
	SCRIPT_NAME: _target ? OR fullpath of the CGI itself?
	SERVER_NAME: What the client connected to, using Host field or default

Easy Copy:

	QUERY_STRING: present in the request
	REMOTE_ADDR: IPv4 address of client
	REQUEST_METHOD: from Request
	SERVER_PORT: from Client or Server parent class
	SERVER_PROTOCOL: from request

Header-Fields:

	PROTOCOL_SPECIFIC_META_VARIABLES
	HTTP_*
	From HeaderField of the request

Hardcoded:

	GATEWAY_INTERFACE = CGI/1.1
	SERVER_SOFTWARE = custom server name ("Plebserv Reforged")
*/
void CgiHandler::generateMetaVariables(const Request& request)
{
	/* To Generate */
	// TODO: REMOTE_HOST, PATH_TRANSLATED
	metaVariableContent(request);

	_meta_variables.push_back(MetaVariableType("SCRIPT_NAME", _target.c_str()));
	// TODO: SERVER_NAME: Check SERVER_NAMES in the ResolvedServer: use Host to determine this
	// Right now it's the IP the client connection to
	_meta_variables.push_back(MetaVariableType("SERVER_NAME", request.interface_addr.first));


	/* Easy Copy */
	_meta_variables.push_back(MetaVariableType(
		"QUERY_STRING", request.query.c_str()));
	_meta_variables.push_back(
		MetaVariableType("REQUEST_METHOD", request.getMethodString().c_str()));
	_meta_variables.push_back(
		MetaVariableType("SERVER_PROTOCOL", request.getProtocolString().c_str()));
	_meta_variables.push_back(MetaVariableType(
		"REMOTE_ADDR", request.address.first));
	_meta_variables.push_back(MetaVariableType(
		"SERVER_PORT", WebservUtility::itoa(request.interface_addr.second)));

	/* Header-Fields */
	metaVariableHeader(request);

	/* Hardcoded */
	_meta_variables.push_back(MetaVariableType("SERVER_SOFTWARE", "Plebserv Remastered"));
	_meta_variables.push_back(MetaVariableType("GATEWAY_INTERFACE", "CGI/1.1"));
}

void CgiHandler::metaVariableContent(const Request& request)
{
	if (request.message_body.size() == 0)
	{
		return;
	}

	_meta_variables.push_back(
		MetaVariableType("CONTENT_LENGTH", WebservUtility::itoa(request.message_body.size())));
	HeaderField::const_pair_type p = request.header_fields.get("content-type");
	if (p.second)
	{
		_meta_variables.push_back(MetaVariableType("CONTENT_TYPE", p.first->second));
	}
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
	if (initializeCgiSender(cgi_fds, fd_table, r) == ERR)
	{
		return ERR;
	}

	if (initializeCgiReader(cgi_fds, fd_table) == ERR)
	{
		if (close(cgi_fds[0]) == ERR)
		{
			syscallError(_FUNC_ERR("close"));
		}
		return ERR;
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
	try {
		_sender = new CgiSender(fds[1], &r);
		fd_table.insertFd(_sender);
	} catch (...) {
		WebservUtility::closePipe(fds);
		delete _sender; // Will be NULL if allocation throw
		_sender = NULL;
		throw;
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

	try {
		_reader = new CgiReader(fds[0]);
		fd_table.insertFd(_reader);
	} catch (...) {
		WebservUtility::closePipe(fds);
		delete _reader;
		_reader = NULL;
		throw;
	}
	return OK;
}

/* Forking, Execve'ing */

int CgiHandler::forkCgi(int* cgi_fds, FdTable& fd_table)
{
	print();
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
	args[1] = strdup(_target.c_str());
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

/* Interfacing Functions */

void CgiHandler::setRootDir(std::string const & root)
{
	_root_dir = root;
}

bool CgiHandler::isChunked(std::string const & http_version) const
{
	if (http_version != "HTTP/1.1")
	{
		return false;
	}

	return _reader->isChunked();
}

bool CgiHandler::evaluateExecutionError()
{
	//TODO: implement functionality
	if ((_reader && _reader->flag == AFdInfo::FILE_ERROR)
		|| (_sender && _sender->flag == AFdInfo::FILE_ERROR))
	{
		return true;
	}
	return false;
}

bool CgiHandler::evaluateExecutionCompletion()
{
	return isComplete();
}

void CgiHandler::setMessageBody(std::string & response_body)
{
	update();
	//TODO: append HeaderFields
	if (_message_body.size() > 0)
	{
		response_body.swap(_message_body);
	}
}

/*
Function's purpose:
	- Check if ERROR occured, CGI child process status (still alive etc)
		IF exited: wait
		IF exited AND sender is INCOMPLETE: BAD_GATEWAY
	- Check if either Sender or Reader is complete: remove from table if so
	- Set completion status if both are complete
		DISCUSS: IF CGI is complete and CGI process is still active (running) : send SIGKILL?

TODO: ERROR handling
	- CGI program exits before it was expected (or crashes)
	- CGI program times out
*/
void CgiHandler::update()
{
	//return if already finished communicating with CGI
	if (isComplete() || isError())
	{
		return;
	}

	if (evaluateExecutionError())
	{
		destroyFds();
		finishCgi(CgiHandler::ERROR, StatusCode::BAD_GATEWAY);
		return;
	}

	if (_reader && _reader->flag == AFdInfo::FILE_COMPLETE)
	{
		// TODO: if sending a CHUNKED Message, then we should APPEND only if it's actively reading
		// HeaderField should just be swapped once it's parsed in that case (add HEADER_COMPLETE)
		_message_body.swap(_reader->getBody());
		_header.swap(_reader->getHeader());
		_reader->setToErase();
		_reader = NULL;
	}

	if (_sender && _sender->flag == AFdInfo::FILE_COMPLETE)
	{
		_sender->setToErase();
		_sender = NULL;
	}

	if (isComplete())
	{
		finishCgi(CgiHandler::COMPLETE, StatusCode::STATUS_OK);
	}
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
	return _sender == NULL && _reader == NULL;
}

bool CgiHandler::isError() const
{
	return _status == CgiHandler::ERROR;
}

bool CgiHandler::isReadyToWrite() const
{
	return isComplete();
}

void CgiHandler::setResponseData(std::string & response_body, HeaderField & response_header)
{
	// This is done only once
	if (_header.size() > 0)
	{
		for (HeaderField::iterator it = _header.begin(); it != _header.end(); ++it)
		{
			response_header[it->first] = it->second;
		}
		_header.clear();
	}

	// Append for chunked?
	response_body.swap(_message_body);
}

void CgiHandler::clearContent()
{
	_message_body.clear();
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

const std::string& CgiHandler::getContent() const
{
	return _message_body;
}

const HeaderField& CgiHandler::getHeaderField() const
{
	return _header;
}

int CgiHandler::getStatusCode() const
{
	return _status_code;
}

CgiHandler::Status CgiHandler::getStatus() const
{
	return _status;
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
