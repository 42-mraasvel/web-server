#include "ResponseGenerator.hpp"
#include "settings.hpp"
#include "utility/utility.hpp"
#include "utility/status_codes.hpp"
#include "Response.hpp"
#include "FileHandler.hpp"
#include "CgiHandler.hpp"

ResponseGenerator::ResponseGenerator() {}

void	ResponseGenerator::generateString(Response & response)
{
	evaluateEncoding(response);

	switch(response.encoding)
	{
		case Response::CHUNKED:
			return generateChunkedResponse(response);
		case Response::NOT_CHUNKED:
			return generateUnchunkedResponse(response);
		default :
			return ;
	}
}

void	ResponseGenerator::evaluateEncoding(Response & response)
{
	if (response.encoding == Response::UNDEFINED)
	{
		if (response.status != Response::COMPLETE)
		{
			if (isReadyToBeChunked(response))
			{
				response.encoding = Response::CHUNKED;
			}
		}
		else
		{
			if (isReadyToBeChunked(response))
			{
				response.encoding = Response::CHUNKED;
			}
			else
			{
				response.encoding = Response::NOT_CHUNKED;
			}
		}
	}
}

bool	ResponseGenerator::isReadyToBeChunked(Response const & response) const
{
	return response.http_version == "HTTP/1.1"
				&& response.message_body.size() >= CHUNK_THRESHOLD;
}

void	ResponseGenerator::generateUnchunkedResponse(Response & response)
{
	setHeaderPart(response);
	appendMessageBody(response);
}

void	ResponseGenerator::generateChunkedResponse(Response & response)
{
	if (!response.header_part_set)
	{
		response.status_code = response.handler->getStatusCode();
		setHeaderPart(response);
	}
	else
	{
		encodeMessageBody(response);
		appendMessageBody(response);
	}
}

void	ResponseGenerator::setHeaderPart(Response & response)
{
	setStringStatusLine(response);
	setHeaderField(response);
	setStringHeaderField(response);
	response.string_to_send = response.string_status_line + NEWLINE
							+ response.string_header_field + NEWLINE;
	response.header_part_set = true;
}

void	ResponseGenerator::setStringStatusLine(Response & response)
{
	response.string_status_line = response.http_version + " "
							+ WebservUtility::itoa(response.status_code) + " "
							+ StatusCode::getStatusMessage(response.status_code);
}

void	ResponseGenerator::setHeaderField(Response & response)
{
	setDate(response);
	setConnection(response);
	setLocation(response);
	setRetryAfter(response);
	setAllow(response);
	setTransferEncodingOrContentLength(response);
	setContentType(response);
	response.handler->setSpecificHeaderField(response.header_fields, response.content_type_fixed);
}

void	ResponseGenerator::setDate(Response & response)
{
	char		buf[1000];
	time_t		now = time(0);
	struct tm	tm = *gmtime(&now);
	strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S %Z", &tm);
	
	response.header_fields["Data"] = std::string(buf);
}

void	ResponseGenerator::setConnection(Response & response)
{
	if (response.close_connection)
	{
		response.header_fields["Connection"] = "close";
	}
	else
	{
		response.header_fields["Connection"] = "keep-alive";
	}
}

void	ResponseGenerator::setLocation(Response & response)
{
	if (response.status_code == StatusCode::CREATED)
	{
		response.header_fields["Location"] = response.request_target;
	}
	else if (StatusCode::isStatusCode3xx(response.status_code))
	{
		response.header_fields["Location"] = response.effective_request_uri;
	}
}

void	ResponseGenerator::setRetryAfter(Response & response)
{
	if (StatusCode::isStatusCode3xx(response.status_code))
	{
		response.header_fields["Retry-After"] = WebservUtility::itoa(RETRY_AFTER_SECONDS);
	}
}

void	ResponseGenerator::setAllow(Response & response)
{
	if (response.status_code == StatusCode::METHOD_NOT_ALLOWED && response.config_info.resolved_location)
	{
		std::string	value;
		for (method_type::const_iterator it = response.config_info.resolved_location->_allowed_methods.begin(); it != response.config_info.resolved_location->_allowed_methods.end(); ++it)
		{
			value.append(*it + ", ");
		}
		if (!value.empty())
		{
			value.erase(value.size() - 2, 2);
		}
		response.header_fields["Allow"] = value;
	}
}

void	ResponseGenerator::setTransferEncodingOrContentLength(Response & response)
{
	if (response.encoding == Response::CHUNKED)
	{
		response.header_fields["Transfer-Encoding"] = "chunked";
	}
	else
	{
		setContentLength(response);
	}
}

void	ResponseGenerator::setContentLength(Response & response)
{
	if (!StatusCode::isStatusCodeNoMessageBody(response.status_code))
	{
		response.header_fields["Content-Length"] = WebservUtility::itoa(response.message_body.size());
	}
}

void	ResponseGenerator::setContentType(Response & response)
{
	if (response.config_info.result == ConfigInfo::AUTO_INDEX_ON
		&& response.status_code == StatusCode::STATUS_OK)
	{
		response.header_fields["Content-Type"] = "text/html";
		response.content_type_fixed = true;
	}
	else if (!response.message_body.empty())
	{
		response.header_fields["Content-Type"] = "text/plain;charset=UTF-8";
	}
}

void	ResponseGenerator::setStringHeaderField(Response & response)
{
	for (header_iterator i = response.header_fields.begin(); i != response.header_fields.end(); ++i)
	{
		response.string_header_field += (i->first + ": " + i->second + NEWLINE);
	}
}

void	ResponseGenerator::appendMessageBody(Response & response)
{
	response.string_to_send.append(response.message_body);
	response.message_body.clear();
}

void	ResponseGenerator::encodeMessageBody(Response & response)
{
	if (!response.message_body.empty())
	{
		std::string chunk_size = WebservUtility::itoa(response.message_body.size(), 16) + NEWLINE;
		response.message_body.insert(0, chunk_size);
		response.message_body.append(NEWLINE);
	}
	if (response.status == Response::COMPLETE)
	{
		response.message_body.append(CHUNK_TAIL);
	}
}
