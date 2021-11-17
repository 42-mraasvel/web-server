#pragma once

# include "parser/HttpRequestParser.hpp"
# include <queue>
# include <string>

struct Request;

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
		RequestHandler();
		~RequestHandler();

		int parse(std::string const & buffer);
		Request* getNextRequest();

	private:

		void newRequest();
		void setErrorRequest();
		void completeRequest();

	private:

		Request* _request;
		RequestQueueType _requests;
		HttpRequestParser _parser;
};
