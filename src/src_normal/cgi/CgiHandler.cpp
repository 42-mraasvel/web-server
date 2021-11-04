#include "CgiHandler.hpp"
#include "settings.hpp"
#include "utility/utility.hpp"

#define CGI_EXTENSION ".py"

#ifdef __linux__
#define SCRIPT_PATH "/usr/bin/python3"
#else
#define SCRIPT_PATH "/Users/mraasvel/.brew/bin/python3"
#endif

#define SCRIPT_NAME "python3"

// Configuration syntax: CGI .py /usr/bin/python3

CgiHandler::CgiHandler()
: _status(CgiHandler::INCOMPLETE) {}

/*
Precondition: target always starts with '/'

Component analysis:
	/c1/c2/c3/c4
	If component ends in EXTENSION: true
*/

bool CgiHandler::isCgi(Request* request) {

	const std::string& target = request->target_resource;

	std::size_t index = 0;
	while (true) {
		// Find the end of component: ('/') or std::string::npos
		std::size_t end = target.find("/", index + 1);
		if (WebservUtility::stringEndsWith(target, CGI_EXTENSION, index, end)) {

			//TODO: _target and PATH_INFO should be the full_path, so SERVER_ROOT/current
			_target = target.substr(0, end);
			if (end != std::string::npos) {
				_meta_variables.push_back(MetaVariableType("PATH_INFO", target.substr(end)));
			} else {
				//TODO: should this be "/" or "" (EMPTY) ?
				_meta_variables.push_back(MetaVariableType("PATH_INFO", ""));
			}
			_meta_variables.clear();

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

2. Open pipes to connect STDIN and STDOUT
3. Fork and execute the script, store the PID internally
4. Close unused pipe ends
*/
int CgiHandler::execute(Request* request)
{
	// generateMetaVariables();
	printf("-- Executing CGI --\n");
	_message_body = "1234";
	_status = COMPLETE;

	print();
	_meta_variables.clear();
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
