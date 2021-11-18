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

/*
1. Validate Header
2. Determine if the connection should be closed
3. Resolve configuration
4. Post configuration request validation
5. Content-Length (maybe in HttpRequestParser?)
6. Check CONTINUE
*/

int RequestHeaderProcessor::process(Request & request)
{
	if (!_request_validator.isRequestValidPreConfig(request))
	{
		request.close_connection = _request_validator.shouldCloseConnection();
		return setError(_request_validator.getStatusCode());
	}


	determineCloseConnection(request);
	// try {
	// _config_resolver.resolution(request);
	// } catch (...) {
	// 	printf("CONFIG RESOLVER THREW\n");
	// 	throw;
	// }

	// request.config = _config_resolver.getData();

	// if (!_request_validator.isRequestValidPostConfig(request, _config_resolver))
	// {
	// 	return setError(_request_validator.getStatusCode());
	// }

	// check payload body
	// Set CONTINUE here?

	return OK;
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
