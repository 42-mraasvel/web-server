#include "CgiResponseParser.hpp"
#include "settings.hpp"
#include "utility/utility.hpp"
#include <limits>

/*
Checks only the CGI related fields
*/
static bool validCgiField(std::string const & key, std::string const & value,
					HeaderField const & header)
{

	return true;
}

CgiResponseParser::CgiResponseParser()
:
_header_parser(validCgiField, MAX_HEADER_SIZE),
_state(CgiResponseParser::PARSE_HEADER),
_status_code(StatusCode::STATUS_OK) {}

/*
Main parsing function
*/

int CgiResponseParser::parse(std::string const & buffer)
{
	std::size_t index = 0;

	while (index < buffer.size())
	{
		switch (_state)
		{
			case CgiResponseParser::PARSE_HEADER:
				parseHeader(buffer, index);
				break;
			case CgiResponseParser::PARSE_CONTENT:
				parseContent(buffer, index);
				break;
			case CgiResponseParser::COMPLETE:
				return OK;
			case CgiResponseParser::ERROR:
				std::cerr << "CgiResponseParser error: " << getStatusCode() << std::endl;
				return ERR;
		}
	}
	return OK;
}

/*
Wrapper parsing functions
*/

void CgiResponseParser::parseHeader(std::string const & buffer,
								std::size_t & index)
{
	if (_header_parser.parse(buffer, index) == ERR)
	{
		// TODO: determine if this should always be BAD_GATEWAY, or HEADER_TOO_LARGE for example?
		setError(StatusCode::BAD_GATEWAY);
	}
	else if (_header_parser.isComplete())
	{
		setContentParsing();
	}
}

void CgiResponseParser::setContentParsing()
{
	HeaderField::const_pair_type length = _header_parser.getHeaderField().get("Content-Length");

	if (length.second == true)
	{
		_content_parser.setContentLength(WebservUtility::strtoul(length.first->second));
	}
	else
	{
		_content_parser.setContentLength(std::numeric_limits<std::size_t>::max());
	}
	setState(CgiResponseParser::PARSE_CONTENT);
}

void CgiResponseParser::parseContent(std::string const & buffer,
								std::size_t & index)
{
	if (_content_parser.parse(buffer, index) == ERR)
	{
		//TODO: BAD_GATEWAY or PAYLOAD_TOO_LARGE?
		setError(StatusCode::BAD_GATEWAY);
	}
	else if (_content_parser.isComplete())
	{
		setState(CgiResponseParser::COMPLETE);
	}
}

/*
Interface functions
*/

HeaderField& CgiResponseParser::getHeader()
{
	return _header_parser.getHeaderField();
}

std::string& CgiResponseParser::getContent()
{
	return _content_parser.getContent();
}

void CgiResponseParser::reset()
{
	_state = CgiResponseParser::PARSE_HEADER;
	_status_code = StatusCode::STATUS_OK;
	_header_parser.reset();
	_content_parser.reset();
}

int CgiResponseParser::getStatusCode() const
{
	return _status_code;
}

int CgiResponseParser::setError(int status)
{
	_state = CgiResponseParser::ERROR;
	_status_code = status;
	return ERR;
}

void CgiResponseParser::setState(State state)
{
	_state = state;
}

bool CgiResponseParser::isError() const
{
	return _state == CgiResponseParser::ERROR;
}

bool CgiResponseParser::isComplete() const
{
	return _state == CgiResponseParser::COMPLETE;
}
