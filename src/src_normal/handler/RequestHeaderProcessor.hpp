#pragma once

# include "handler/ConfigResolver.hpp"
# include "handler/RequestValidator.hpp"

struct Request;

/*
Functionality:

	Processes the header and sets the following Request data:
		
		- Configuration resolution
		- Close flag

	If an error is encountered, returns ERR and stores relevant StatusCode

Flow:
	1. Validates headerFields, sets connection flag if necessary
	2. Determines if the connection should be closed
	3. Resolves configuration
	4. Checks if the method is allowed
	5. Checks if the origin-server should send a CONTINUE response
*/

class RequestHeaderProcessor
{
	public:

		int process(Request & request);
		int getStatusCode() const;

	private:
	
		int setError(int code);

	private:
		int _status_code;
		RequestValidator _request_validator;
		ConfigResolver _config_resolver;
};
