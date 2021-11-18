#include "RequestHeaderProcessor.hpp"
#include "settings.hpp"

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
		// Check if connection should be closed
		request.close_connection = _request_validator.shouldCloseConnection();
		return setError(_request_validator.getStatusCode());
	}
	return OK;
}

int RequestHeaderProcessor::getStatusCode() const
{
	return _status_code;
}

int RequestHeaderProcessor::setError(int code)
{
	_status_code = code;
	return ERR;
}
