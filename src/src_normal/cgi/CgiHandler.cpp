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
		checkCgi();
	}
}

/*
Precondition: target always starts with '/'

Component analysis:
	/c1/c2/c3/c4
	If component ends in EXTENSION: true
*/

bool CgiHandler::isCgi(const Request& request) {

	const std::string& target = request.target_resource;

	std::size_t index = 0;
	while (true) {
		// Find the end of component: ('/') or std::string::npos
		std::size_t end = target.find("/", index + 1);
		if (WebservUtility::stringEndsWith(target, CGI_EXTENSION, index, end)) {

			//TODO: _target should be the FULL path (?) /SERVER_ROOT/_target
			_target = target.substr(0, end);
			if (end != std::string::npos) {
				_meta_variables.push_back(MetaVariableType("PATH_INFO", target.substr(end)));
			} else {
				//TODO: should this be "/" or "" (EMPTY) ?
				_meta_variables.push_back(MetaVariableType("PATH_INFO", ""));
			}

			_status = CgiHandler::INCOMPLETE;
			return true;
		} else if (end == std::string::npos) {
			break;
		}
		index = end;
	}
	return false;
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
	printf("-- Executing CGI --\n");

	/* 1. Preparation */
	//TODO: replace with configuration script (matched script)
	_script = SCRIPT_PATH;
	if (!scriptCanBeExecuted())
	{
		finishResponse(COMPLETE, StatusCode::BAD_GATEWAY);
		return ERR;
	}

	printf("ROOT DIR: %s\n", _root_dir.c_str());
	// TODO: replace with _root_dir
	_target = SERVER_ROOT + _target;
	generateMetaVariables(request);

	/* 2. Open pipes, create FdClasses */
	int fds[2] = {-1, -1};
	if (initializeCgiConnection(fds, fd_table, request) == ERR)
	{
		finishResponse(COMPLETE, StatusCode::INTERNAL_SERVER_ERROR);
		return ERR;
	}

	/* 3. Fork */
	if (forkCgi(fds, fd_table) == ERR)
	{
		finishResponse(COMPLETE, StatusCode::INTERNAL_SERVER_ERROR);
		WebservUtility::closePipe(fds);
		return ERR;
	}

	/* 4. Close unused pipes */
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
	// TODO: REMOTE_HOST, SERVER_NAME, PATH_TRANSLATED
	metaVariableContent(request);
	// metaVariablePathTranslated();

	// TODO: check if Correct interpretation of SCRIPT_NAME variable
	_meta_variables.push_back(MetaVariableType("SCRIPT_NAME", _target.c_str()));
	// TODO: Should be the server name the client connected to (host-header-field)
	_meta_variables.push_back(MetaVariableType("SERVER_NAME", "127.0.0.1"));
	

	/* Easy Copy */
	_meta_variables.push_back(MetaVariableType("QUERY_STRING", request.query.c_str()));
	_meta_variables.push_back(
		MetaVariableType("REQUEST_METHOD", request.getMethodString().c_str()));
	_meta_variables.push_back(
		MetaVariableType("SERVER_PROTOCOL", request.getProtocolString().c_str()));

	// TODO: REMOTE_ADDR, SERVER_PORT from socket information, REMOTE_ADDR from accept information
	_meta_variables.push_back(MetaVariableType("REMOTE_ADDR", "127.0.0.1"));
	_meta_variables.push_back(MetaVariableType("SERVER_PORT", "80"));

	/* Header-Fields */
	// TODO: copy header-field values

	/* Hardcoded */
	_meta_variables.push_back(MetaVariableType("SERVER_SOFTWARE", "Plebserv Reforged"));
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

/*
TODO: ERROR handling
	- CGI program exits before it was expected (or crashes)
	- CGI program times out
	- CGI program returns an invalid CGI response
*/
bool CgiHandler::evaluateExecutionError()
{
	//TODO: implement functionality
	return false;
}

bool CgiHandler::evaluateExecutionCompletion()
{
	return isComplete();
}

bool CgiHandler::isReadyToWrite() const
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

TODO: ERROR checks and flag should be done here (evaluateExecutionError)
*/
void CgiHandler::update()
{
	//return if already finished communicating with CGI
	if (_status == CgiHandler::COMPLETE)
	{
		return;
	}

	if (_reader && _reader->flag == AFdInfo::FILE_COMPLETE)
	{
		_message_body = _reader->getBody();
		_reader->clearBody();
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
		checkCgi();
		finishResponse(CgiHandler::COMPLETE, StatusCode::STATUS_OK);
	}
}

int CgiHandler::checkCgi()
{
	if (_cgi_pid == -1)
	{
		return OK;
	}

	int status;
	if (cleanCgi(&status) == ERR)
	{
		return ERR;
	}
	// TODO: Check exit status, crash, signal etc
	_cgi_pid = -1;
	return OK;
}

int CgiHandler::cleanCgi(int* status)
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
	return
		(_sender == NULL || _sender->flag == AFdInfo::FILE_COMPLETE) && 
		(_reader == NULL || _reader->flag == AFdInfo::FILE_COMPLETE);
}

void CgiHandler::clearContent()
{
	_message_body.clear();
}

void CgiHandler::finishResponse(Status status, int code)
{
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
