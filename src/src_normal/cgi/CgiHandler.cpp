#include "CgiHandler.hpp"
#include "settings.hpp"
#include "utility/utility.hpp"
#include "utility/macros.hpp"
#include "CgiSender.hpp"
#include "CgiReader.hpp"
#include <unistd.h>

#define CGI_EXTENSION ".py"

#ifdef __linux__
#define SCRIPT_PATH "/usr/bin/python3"
#else
#define SCRIPT_PATH "/Users/mraasvel/.brew/bin/python3"
#endif

#ifdef __linux__
#define SERVER_ROOT "/home/mraasvel/work/codam/webserv-pyxis/page_sample"
#else
#define SERVER_ROOT "/Users/mraasvel/work/codam/webserv-pyxis/page_sample"
#endif /* __linux__ */

// Configuration syntax: CGI .py /usr/bin/python3

CgiHandler::CgiHandler()
: _status(CgiHandler::INCOMPLETE), _sender(NULL), _reader(NULL) {}

/*
Precondition: target always starts with '/'

Component analysis:
	/c1/c2/c3/c4
	If component ends in EXTENSION: true
*/

bool CgiHandler::isCgi(const Request* request) {

	const std::string& target = request->target_resource;

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
int CgiHandler::execute(Request* request, FdTable& fd_table)
{
	printf("-- Executing CGI --\n");

	/* 1. Preparation */
	_script = SCRIPT_PATH;
	generateMetaVariables(request);

	/* 2. Open pipes, create FdClasses */
	int fds[2];
	if (initializeCgiConnection(fds, fd_table, request) == ERR)
	{
		return ERR;
	}

	_target = SERVER_ROOT + _target;
	/* 3. Fork */
	// executeCgi(fds);


	/* 4. Close unused pipes */
	WebservUtility::closePipe(fds);

	_status = COMPLETE;
	print();
	_meta_variables.clear();
	return OK;
}

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
void CgiHandler::generateMetaVariables(const Request* request)
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
	_meta_variables.push_back(MetaVariableType("QUERY_STRING", request->query.c_str()));
	_meta_variables.push_back(
		MetaVariableType("REQUEST_METHOD", request->getMethodString().c_str()));
	_meta_variables.push_back(
		MetaVariableType("SERVER_PROTOCOL", request->getProtocolString().c_str()));

	// TODO: REMOTE_ADDR, SERVER_PORT from socket information, REMOTE_ADDR from accept information
	_meta_variables.push_back(MetaVariableType("REMOTE_ADDR", "127.0.0.1"));
	_meta_variables.push_back(MetaVariableType("SERVER_PORT", "80"));

	/* Header-Fields */
	// TODO: copy header-field values

	/* Hardcoded */
	_meta_variables.push_back(MetaVariableType("SERVER_SOFTWARE", "Plebserv Reforged"));
	_meta_variables.push_back(MetaVariableType("GATEWAY_INTERFACE", "CGI/1.1"));
}

void CgiHandler::metaVariableContent(const Request* request)
{
	if (request->message_body.size() == 0)
	{
		return;
	}

	_meta_variables.push_back(
		MetaVariableType("CONTENT_LENGTH", WebservUtility::itoa(request->message_body.size())));
	HeaderField::const_pair_type p = request->header_fields.get("content-type");
	if (p.second)
	{
		_meta_variables.push_back(MetaVariableType("CONTENT_TYPE", p.first->second));
	}
}

/*
Initializes the CgiReader, CgiSender classes
Stores the FDs used inside of the CGI in rfds
*/
int CgiHandler::initializeCgiConnection(int* cgi_fds, FdTable& fd_table, Request* r)
{
	if (initializeCgiReader(cgi_fds, fd_table) == ERR)
	{
		return ERR;
	}

	if (initializeCgiSender(cgi_fds, fd_table, r) == ERR)
	{
		return ERR;
	}

	return OK;
}

int CgiHandler::initializeCgiSender(int* cgi_fds, FdTable& fd_table, Request* r)
{
	int fds[2];

	if (pipe(fds) == ERR)
	{
		return syscallError(_FUNC_ERR("pipe"));
	}

	// Sender needs the WRITE end of the pipe, so it gives the READ end to the CGI
	cgi_fds[0] = fds[0];

	// Instantiate the CgiSender with the WRITE end of the pipe and add it to the FdTable
	_sender = new CgiSender(fds[1], r);
	fd_table.insertFd(_sender);
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
	_reader = new CgiReader(fds[0]);
	fd_table.insertFd(_reader);
	return OK;
}

bool CgiHandler::isComplete() const
{
	return _status == COMPLETE;
}

void CgiHandler::clearContent()
{
	_message_body.clear();
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
	for (MetaVariableContainerType::const_iterator it = _meta_variables.begin();
		it != _meta_variables.end(); ++it)
	{
		printf("%s: %s\n", it->first.c_str(), it->second.c_str());
	}
}
