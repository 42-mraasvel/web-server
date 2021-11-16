#pragma once

# include "parser/HttpRequestParser.hpp"
# include <queue>
# include <string>

class Request;

/*
Uses the HttpRequestParser to parse the buffer
If the Request is COMPLETE, it adds the request to the RequestQueue
Completely parses the buffer

-- Interface --

	parse(buffer);
	getNextRequest();
*/
class RequestHandler
{
	public:

		typedef std::queue<Request*> RequestQueueType;

	public:

		int parse(std::string const & buffer);
		Request* getNextRequest();

	private:

		Request* _request;
		RequestQueueType _requests;
		HttpRequestParser _parser;
};
