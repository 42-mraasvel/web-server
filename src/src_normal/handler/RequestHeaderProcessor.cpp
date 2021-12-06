#include "RequestHeaderProcessor.hpp"
#include "settings.hpp"
#include "utility/utility.hpp"
#include "utility/status_codes.hpp"

int RequestHeaderProcessor::getStatusCode() const
{
	return _status_code;
}

int RequestHeaderProcessor::setError(int code)
{
	_status_code = code;
	return ERR;
}

RequestHeaderProcessor::RequestHeaderProcessor(MapType const * config_map)
: _config_resolver(config_map) {}

/*
1. Validate Header
2. Determine if the connection should be closed
3. Resolve configuration
4. Post configuration request validation
*/

int RequestHeaderProcessor::process(Request & request)
{
	if (!_request_validator.isRequestValidPreConfig(request))
	{
		request.close_connection = _request_validator.shouldCloseConnection();
		return setError(_request_validator.getStatusCode());
	}

	determineCloseConnection(request);
	try {
		_config_resolver.resolution(request, getHostString(request.header_fields));
	} catch (const std::exception& e) {
		generalError("%s: caught exception: %s\n", _FUNC_ERR("ConfigResolver").c_str(), e.what());
		throw e;
	}
	request.config_info = _config_resolver.getConfigInfo();
	if (request.config_info.result == ConfigInfo::NOT_FOUND)
	{
		return setError(StatusCode::NOT_FOUND);
	}

	if (!_request_validator.isRequestValidPostConfig(request))
	{
		return setError(_request_validator.getStatusCode());
	}
	return OK;
}

void RequestHeaderProcessor::processError(Request & request)
{
	if (request.config_info.resolved_server != NULL)
	{
		return;
	}
	request.request_target = "";
	request.method = Method::GET;
	_config_resolver.resolution(request, "");
	request.config_info = _config_resolver.getConfigInfo();
}

/*
Determine close connection
*/

void RequestHeaderProcessor::determineCloseConnection(Request & request)
{
	HeaderField::const_pair_type connection = request.header_fields.get("connection");
	if (connection.second)
	{
		if (WebservUtility::caseInsensitiveEqual(connection.first->second, "close"))
		{
			request.close_connection = true;
		}
	}
	else if (request.minor_version == 0)
	{
		request.close_connection = true;
	}
}

std::string RequestHeaderProcessor::getHostString(HeaderField const & header)
{
	HeaderField::const_pair_type host = header.get("host");
	if (!host.second)
	{
		return "";
	}
	return WebservUtility::trimPort(host.first->second);
}
