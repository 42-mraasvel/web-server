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
		typedef std::queue<Request*> 		RequestQueueType;
		typedef	ConfigResolver::MapType		MapType;
		typedef	ConfigResolver::AddressType	AddressType;


	public:
		RequestHandler(AddressType address, MapType const * config_map);
		~RequestHandler();

		int parse(std::string const & buffer);
		Request* getNextRequest();

	private:

		void newRequest();
		void setErrorRequest();
		void completeRequest();

		bool isContinueResponse(Request const & request) const;
		void newContinueRequest();

	private:

		AddressType _address;
		Request* _request;
		RequestQueueType _requests;
		HttpRequestParser _parser;
};
