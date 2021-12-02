#include "ResponseGenerator.hpp"
#include "settings.hpp"
#include "utility/utility.hpp"
#include "utility/status_codes.hpp"
#include "FileHandler.hpp"
#include "CgiHandler.hpp"

ResponseGenerator::ResponseGenerator():
_status_code(0),
_header_part_set(false)
{}

/***********************************************/
/****** (Client::write) generate responses *****/
/***********************************************/

void	ResponseGenerator::generateResponse(ResponseInfo const & info, iHandler* handler)
{
	if (info.encoding == ResponseInfo::CHUNKED)
	{
		doChunked(info, handler);
	}
	else
	{
		if (info.status == ResponseInfo::COMPLETE)
		{
			noChunked(info, handler);
		}
	}
}

void	ResponseGenerator::noChunked(ResponseInfo const & info, iHandler* handler)
{
	setHeaderPart(info, handler);
	_string_to_send.append(message_body);
	message_body.clear();
}

void	ResponseGenerator::doChunked(ResponseInfo const & info, iHandler* handler)
{
	if (!_header_part_set)
	{
		setHeaderPart(info, handler);
	}
	if (_header_part_set)
	{
		encodeMessageBody(info.status);
		_string_to_send.append(message_body);
		message_body.clear();
	}
}

void	ResponseGenerator::encodeMessageBody(ResponseInfo::Status status)
{
	if (!message_body.empty())
	{
		std::string chunk_size = WebservUtility::itoa(message_body.size(), 16) + NEWLINE;
		message_body.insert(0, chunk_size);
		message_body.append(NEWLINE);
	}
	if (status == ResponseInfo::COMPLETE)
	{
		message_body.append(CHUNK_TAIL);
	}
}

void	ResponseGenerator::setHeaderPart(ResponseInfo const & info, iHandler* handler)
{
	setStatusCode(info, handler);
	setStringStatusLine(info.http_version);
	setHeaderField(info, handler);
	setStringHeaderField();
	_string_to_send = _string_status_line + NEWLINE
					+ _string_header_field + NEWLINE;

	_header_part_set = true;
}

void	ResponseGenerator::setStatusCode(ResponseInfo const & info, iHandler* handler)
{
	if (info.status != ResponseInfo::COMPLETE)
	{
		_status_code = handler->getStatusCode();
	}
	else
	{
		_status_code = info.status_code;
	}
}

void	ResponseGenerator::setStringStatusLine(std::string const & http_version)
{
	_string_status_line = http_version + " "
							+ WebservUtility::itoa(_status_code) + " "
							+ StatusCode::getStatusMessage(_status_code);
}

void	ResponseGenerator::setHeaderField(ResponseInfo const & info, iHandler* handler)
{
	setDate();
	setConnection(info.close_connection);
	setLocation(info);
	setRetryAfter();
	setAllow(info.config_info);
	setTransferEncodingOrContentLength(info.encoding);
	setContentType(info.config_info.result);
	handler->setSpecificHeaderField(_header_fields);
}

void	ResponseGenerator::setDate()
{
	char		buf[1000];
	time_t		now = time(0);
	struct tm	tm = *gmtime(&now);
	strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S %Z", &tm);
	
	_header_fields["Data"] = std::string(buf);
}

void	ResponseGenerator::setConnection(bool close_connection)
{
	if (close_connection)
	{
		_header_fields["Connection"] = "close";
	}
	else
	{
		_header_fields["Connection"] = "keep-alive";
	}
}

void	ResponseGenerator::setLocation(ResponseInfo const & info)
{
	if (_status_code == StatusCode::CREATED)
	{
		// TODO: to check with Maarten how CGI is set?
		_header_fields["Location"] = info.request_target;
	}
	else if (StatusCode::isStatusCode3xx(_status_code))
	{
		_header_fields["Location"] = info.effective_request_uri;
	}
}

void	ResponseGenerator::setRetryAfter()
{
	if (StatusCode::isStatusCode3xx(_status_code))
	{
		_header_fields["Retry-After"] = WebservUtility::itoa(RETRY_AFTER_SECONDS);
	}
}

void	ResponseGenerator::setAllow(ConfigInfo const & config_info)
{
	if (_status_code == StatusCode::METHOD_NOT_ALLOWED && config_info.resolved_location)
	{
		std::string	value;
		for (method_type::const_iterator it = config_info.resolved_location->_allowed_methods.begin(); it != config_info.resolved_location->_allowed_methods.end(); ++it)
		{
			value.append(*it + ", ");
		}
		if (!value.empty())
		{
			value.erase(value.size() - 2, 2);
		}
		_header_fields["Allow"] = value;
	}
}

void	ResponseGenerator::setTransferEncodingOrContentLength(ResponseInfo::Encoding encoding)
{
	if (encoding == ResponseInfo::CHUNKED)
	{
		_header_fields["Transfer-Encoding"] = "chunked";
	}
	else
	{
		setContentLength();
	}
}

void	ResponseGenerator::setContentLength()
{
	if (!StatusCode::isStatusCodeNoMessageBody(_status_code))
	{
		_header_fields["Content-Length"] = WebservUtility::itoa(message_body.size());
	}
}

void	ResponseGenerator::setContentType(ConfigInfo::ConfigResult result)
{
	if (result == ConfigInfo::AUTO_INDEX_ON)
	{
		_header_fields["Content-Type"] = "text/html";
		return ;
	}
}

void	ResponseGenerator::setStringHeaderField()
{
	for (header_iterator i = _header_fields.begin(); i !=_header_fields.end(); ++i)
	{
		_string_header_field += (i->first + ": " + i->second + NEWLINE);
	}
}

void	ResponseGenerator::appendString(std::string & append_to)
{
	append_to.append(_string_to_send);
	_string_to_send.clear();
}
