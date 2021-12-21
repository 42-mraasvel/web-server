#pragma once

# include "config/ConfigResolver.hpp"
# include "request/RequestValidator.hpp"

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
*/

class RequestHeaderProcessor
{
	public:
		typedef	ConfigResolver::MapType		MapType;

	public:
		RequestHeaderProcessor(MapType const * config_map);

		int process(Request & request);
		int processPostParsing(Request & request);
		void processError(Request & request, int status_code);
		int getStatusCode() const;

	private:
	
		int setError(int code);

	/* Determine close connection */
		bool isTooLargeStatusCode(int status_code);
		void determineCloseConnection(Request & request);

	/* Config Resolution */

		std::string getHostString(HeaderField const & header);

	private:
		int _status_code;
		RequestValidator _request_validator;
		ConfigResolver _config_resolver;
};
