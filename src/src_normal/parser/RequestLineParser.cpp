#include "RequestLineParser.hpp"
#include "settings.hpp"
#include "utility/status_codes.hpp"
#include "ParserUtils.hpp"

#define MAX_REQUESTLINE_SIZE 4096

RequestLineParser::RequestLineParser()
: _state(RequestLineParser::PARSE) {}

/*
to implement:
	int parseRequestLine();
	int parseSpace();
	int parseMethod();
	int parseTargetResource();
	bool skipAbsolutePath();
	bool skipQuery();
	int parseVersion();
	bool parseMajorVersion();
	bool parseMinorVersion();
*/

/* Public Interface Functions */

int RequestLineParser::parse(std::string const & buffer, std::size_t & index, Request & request)
{
	if (!hasEndLine(buffer, index))
	{
		return appendLeftover(buffer, index);
	}

	WebservUtility::skipEndLine(buffer, index);
	// return parseRequestLine(buffer, index, request);
}

bool RequestLineParser::isComplete() const
{
	return _state == RequestLineParser::COMPLETE;
}

bool RequestLineParser::isError() const
{
	return _state == RequestLineParser::ERROR;
}

int RequestLineParser::getStatusCode() const
{
	return _status_code;
}

/* Private Methods */

bool RequestLineParser::hasEndLine(std::string const & buffer, std::size_t index)
{
	index = WebservUtility::findEndLine(_leftover, buffer, index);
	if (index == std::string::npos)
	{
		return false;
	}
	return true;
}

int RequestLineParser::appendLeftover(std::string const & buffer, std::size_t & index)
{
	if ((_leftover.size() + buffer.size() - index) > MAX_REQUESTLINE_SIZE)
	{
		//TODO: Review what error should be given here
		return setError(StatusCode::URI_TOO_LONG);
	}
	_leftover.append(buffer, index);
	index = buffer.size();
	return OK;
}

int RequestLineParser::setError(int code)
{
	_state = RequestLineParser::ERROR;
	_status_code = code;
	return ERR;
}
