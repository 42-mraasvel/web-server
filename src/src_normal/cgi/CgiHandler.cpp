#include "CgiHandler.hpp"
#include "settings.hpp"

#define CGI_EXTENSION ".py"
#define SCRIPT_PATH "/usr/bin/python3"
#define SCRIPT_NAME "python3"

// Configuration: CGI .py /usr/bin/python3

CgiHandler::CgiHandler()
: _status(CgiHandler::INCOMPLETE) {}

/*
Returns true if the Request is supposed to be of type CGI
Need:
	- Request URI to determine if it contains the file extension
	- Configuration settings to access location CGI options

URI Example:

	/x/y/z.cgi/a/b/c?querya=s=d=fsl%23

	TARGET_RESOURCE = /x/y/z.cgi/a/b/c

	SCRIPT_PATH = /x/y/z.cgi
	QUERY_STRING = querya=s=d=fsl%23
	PATH_INFO = /a/b/c
*/
bool CgiHandler::isCgi(Request* request) const
{
	std::size_t index = request->target_resource.find(CGI_EXTENSION);

	// TODO: check all the .cgi extensions, not just the first one
	index += sizeof(CGI_EXTENSION) - 1;
	if (index != std::string::npos &&
	(index == request->target_resource.size()
	|| request->target_resource[index] == '/'))
	{
		printf("Valid CGI: %s\n", request->target_resource.c_str());
		return true;
	}
	return false;
}

/*
1. Preparation: HeaderChecks, ScriptLocation, MetaVariables
2. Open pipes to connect STDIN and STDOUT
3. Fork and execute the script, store the PID internally
4. Close unused pipe ends
*/
int CgiHandler::execute(Request* request)
{
	printf("-- Executing CGI --\n");
	_message_body = "1234";
	_status = COMPLETE;
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
