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
		typedef SmartPointer<Request>		RequestPointer;
		typedef std::queue<RequestPointer> 	RequestQueueType;
		typedef	ConfigResolver::MapType		MapType;
		typedef	ConfigResolver::AddressType	AddressType;

	private:
		RequestHandler(const RequestHandler& rhs);
		RequestHandler& operator=(const RequestHandler& rhs);

	public:
		RequestHandler(AddressType client, AddressType interface, MapType const * config_map);
		~RequestHandler();

		int parse(std::string const & buffer);
		RequestPointer getNextRequest();
		bool isNextRequestSafe() const;
		std::size_t numRequests() const;
		void clear();
		void newTimeoutRequest();

	private:
		void newRequest();
		void setErrorRequest();
		void completeRequest();

		bool isContinueResponse(Request const & request) const;
		void newContinueRequest();

	private:

		AddressType _client_addr;
		AddressType _interface_addr;
		RequestPointer _request;
		RequestQueueType _requests;
		HttpRequestParser _parser;
};
