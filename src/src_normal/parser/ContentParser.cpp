#include "ContentParser.hpp"
#include "settings.hpp"

ContentParser::ContentParser()
{
	reset();
}

int ContentParser::parse(std::string const & buffer, std::size_t& index)
{
	return parseRawData(buffer, index);
}

std::string& ContentParser::getContent()
{
	return _content;
}

void ContentParser::reset()
{
	_content.clear();
	_content_length = 0;
	_bytes_read = 0;
	_max_size = std::numeric_limits<std::size_t>::max();
	_state = ContentParser::PARSING;
	_status_code = StatusCode::STATUS_OK;
}

/*
if std::numeric_limits<std::size_t>::max(): always parse everything
*/
void ContentParser::setContentLength(std::size_t length)
{
	_content_length = length;
}

void ContentParser::setMaxSize(std::size_t max)
{
	_max_size = max;
}

std::size_t ContentParser::getMaxSize() const
{
	return _max_size;
}

bool ContentParser::isError() const
{
	return _state == ContentParser::ERROR;
}

bool ContentParser::isComplete() const
{
	return _state == ContentParser::COMPLETE;
}

int ContentParser::getStatusCode() const
{
	return _status_code;
}

int ContentParser::parseRawData(std::string const & buffer, std::size_t& index)
{
	std::size_t len = std::min(_content_length - _bytes_read, buffer.size() - index);
	
	if (len + _bytes_read > _max_size)
	{
		return setError(StatusCode::PAYLOAD_TOO_LARGE);
	}

	_content.append(buffer, index, len);
	index += len;
	_bytes_read += len;
	if (_bytes_read == _content_length)
	{
		ContentParser::setComplete();
	}
	return OK;
}

void ContentParser::setComplete()
{
	_state = ContentParser::COMPLETE;
}

int ContentParser::setError(StatusCode::Code code)
{
	_status_code = code;
	_state = ContentParser::ERROR;
	return ERR;
}
