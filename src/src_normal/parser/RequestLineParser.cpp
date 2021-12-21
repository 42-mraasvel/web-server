#include "RequestLineParser.hpp"
#include "settings.hpp"
#include "outputstream/Output.hpp"
#include "utility/status_codes.hpp"
#include "utility/utility.hpp"
#include "ParserUtils.hpp"
#include "http_bnf.hpp"
#include "Request.hpp"

#define MAX_REQUESTLINE_SIZE 4096

RequestLineParser::RequestLineParser()
: _state(RequestLineParser::PARSE), _index(0) {}

/* Public Interface Functions */

/*
1. Store until ENDLINE into LEFTOVER (return if no ENDLINE)
2. Set the index to beyond the RequestLine
3. Parse the requestline
*/
int RequestLineParser::parse(std::string const & buffer, std::size_t & index, Request & request)
{
	std::size_t pos = WebservUtility::findEndLine(_leftover, buffer, index);

	if (_state != PARSE)
	{
		PRINT_WARNING << "RequestLineParser called with invalid state" << std::endl;
		return ERR;
	}

	if (pos == std::string::npos)
	{
		return appendLeftover(buffer, index, buffer.size() - index);
	}

	appendLeftover(buffer, index, pos - index);
	WebservUtility::skipEndLine(buffer, index);
	if (parseRequestLine(request) == ERR)
	{
		return setError(StatusCode::BAD_REQUEST);
	}
	return OK;
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

void RequestLineParser::reset()
{
	_leftover.clear();
	_index = 0;
	_state = RequestLineParser::PARSE;
}

/* Actual Parsing */

int RequestLineParser::parseRequestLine(Request & request)
{
	if (parseMethod(request) != OK)
	{
		return ERR;
	}

	if (parseSpace() != OK)
	{
		return ERR;
	}

	if (parseTargetResource(request) != OK)
	{
		return ERR;
	}

	if (parseSpace() != OK)
	{
		return ERR;
	}

	if (parseVersion(request) != OK)
	{
		return ERR;
	}

	if (parseEndLine() != OK)
	{
		return ERR;
	}

	return setComplete();
}

int RequestLineParser::parseSpace()
{
	if (_leftover[_index] != ' ')
	{
		return ERR;
	}
	++_index;
	return OK;

}

int RequestLineParser::parseMethod(Request & request)
{
	std::size_t start = _index;
	if (!isTokenChar(_leftover[_index]))
	{
		return ERR;
	}

	WebservUtility::skip(_leftover, _index, isTokenChar);
	request.method = Request::getMethodType(_leftover.substr(start, _index - start));
	return OK;
}

int RequestLineParser::parseTargetResource(Request & request)
{
	std::size_t start = _index;
	if (skipAbsolutePath() == ERR) {
		return ERR;
	}
	request.request_target = _leftover.substr(start, _index - start);
	start = _index;
	if (request.request_target.find("..") != std::string::npos) {
		return ERR;
	}
	if (decodeRequestTarget(request.request_target) == ERR) {
		return ERR;
	}
	skipQuery();
	request.query = _leftover.substr(start, _index - start);
	return OK;
}

int RequestLineParser::decodeRequestTarget(std::string & request_target) const
{
	for (std::size_t i = 0; i < request_target.size(); ++i)
	{
		if (request_target[i] == '%')
		{
			if (request_target.size() - 2 < 2 ||
				!isHex(request_target[i + 1]) || !isHex(request_target[i + 2]))
			{
				return ERR;
			}

			request_target[i] = decodePercent(request_target[i + 1], request_target[i + 2]);
			request_target.erase(i + 1, 2);
		}
	}
	return OK;
}

/*
%20
0 = 0 * 16^0
2 = 2 * 16^1
*/
char RequestLineParser::decodePercent(char x, char y) const {
	static const std::string hex_digits(HEXDIG);
	return hex_digits.find(tolower(x)) * 16 + hex_digits.find(tolower(y));
}

int RequestLineParser::skipAbsolutePath()
{
	if (_leftover[_index] != '/')
	{
		return ERR;
	}

	while (_leftover[_index] == '/')
	{
		// NginX skips all slashes
		while (_leftover[_index] == '/')
		{
			++_index;
		}
		if (!isPchar(_leftover[_index]))
		{
			break;
		}
		WebservUtility::skip(_leftover, _index, isPchar);
	}
	return OK;
}

int RequestLineParser::skipQuery()
{
	if (_leftover[_index] != '?') {
		return OK;
	}

	WebservUtility::skip(_leftover, _index, isQueryChar);
	if (_leftover[_index] == '#') {
		++_index;
	}
	return OK;
}

int RequestLineParser::parseVersion(Request & request)
{
	if (_leftover.compare(_index, 5, "HTTP/") != 0)
	{
		return ERR;
	}
	_index += 5;
	if (parseMajorVersion(request) == ERR)
	{
		return ERR;
	}
	if (_leftover[_index] != '.')
	{
		return ERR;
	}
	++_index;

	if (parseMinorVersion(request) == ERR)
	{
		return ERR;
	}

	return OK;
}

int RequestLineParser::parseMajorVersion(Request & request)
{
	if (_leftover[_index] == '0' || !isDigit(_leftover[_index]))
	{
		return ERR;
	}
	std::size_t start = _index;
	WebservUtility::skip(_leftover, _index, isDigit);
	request.major_version = WebservUtility::strtol(_leftover.data() + start);
	return OK;
}

int RequestLineParser::parseMinorVersion(Request & request)
{
	std::size_t start = _index;
	WebservUtility::skip(_leftover, _index, isDigit);
	if (_index - start > 3 || _index - start == 0)
	{
		return ERR;
	}
	request.minor_version = WebservUtility::strtol(_leftover.data() + start);
	return OK;
}

int RequestLineParser::parseEndLine() const
{
	if (_index != _leftover.size())
	{
		return ERR;
	}
	return OK;
}

/* Private Methods */

int RequestLineParser::appendLeftover(std::string const & buffer, std::size_t & index,
									std::size_t len)
{
	if ((_leftover.size() + len) > MAX_REQUESTLINE_SIZE)
	{
		return setError(StatusCode::URI_TOO_LONG);
	}
	_leftover.append(buffer, index, len);
	index += len;
	return OK;
}

int RequestLineParser::setError(int code)
{
	_state = RequestLineParser::ERROR;
	_status_code = code;
	return ERR;
}

int RequestLineParser::setComplete()
{
	_leftover.clear();
	_state = RequestLineParser::COMPLETE;
	return OK;
}
