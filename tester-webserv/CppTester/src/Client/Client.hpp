#pragma once

#include "DataStructures/Request.hpp"
#include "RequestStringGenerator.hpp"
#include "Parser/HttpResponseParser.hpp"
#include "ResponseValidator/ResponseValidator.hpp"
#include "Utility/Timer.hpp"
#include <deque>
#include <poll.h>
#include <memory>
#include <utility>

class ResponseValidator;

class Client {
	public:

		struct Settings {
			public:
				enum Flags {
					NONE			= 0x0000,
					TIMEOUT			= 0x0001, // Close the connection
					PIPELINED		= 0x0002, // Send multiple requests without waiting for the response
					WAIT_FOR_CLOSE	= 0x0004,
				};
			public:
				Settings();

			public:
				short int flags;
				int timeout;
				int pipeline_amount;
				int wait_close;

			private:
				static constexpr int DEFAULT_TIMEOUT = 60;
				static constexpr int DEFAULT_PIPELINE = 10;
				static constexpr int DEFAULT_WAIT_CLOSE = 120;
		};
	
	private:
		enum class State {
			EXECUTING,
			COMPLETE,
			ERROR,
			WAITING
		};

	public:
		typedef std::pair<Request::Pointer, ResponseValidator> RequestPair;
		typedef std::deque<RequestPair> RequestQueue;
		typedef RequestQueue::const_iterator RequestIterator;
		typedef ResponseValidator::ResponseVector ResponseVector;
		typedef std::deque<ResponseVector> ResponseQueue;

	public:
		Client(Settings options);
		~Client();

		static void testRequest(RequestPair request, Settings settings = Settings());
		static void testRequests(const RequestQueue& requests, Address server_addr, Settings settings = Settings());

		bool isComplete() const;
		bool isError() const;
		void handleError(const RequestQueue& requests) const;

	private:
	/* Error Checking Input */
		static bool validRequests(const RequestQueue& requests);
		static bool validRequest(const Request::Pointer request);
		static bool validSettings(const Settings& settings);

	/* Connection */
		int initializeConnection(const Address& server_addr);
		int connectToServer(struct sockaddr_in) const;

		void executeTransaction(RequestQueue requests);
		void prepareTransaction(const RequestQueue& requests);
		bool shouldCloseConnection();
		void executeEvents();

	/* Update Event */
		void update(RequestQueue& requests);
		bool shouldGenerateRequestString(const RequestQueue& requests) const;
		void generateMultipleRequests(const RequestQueue& requests);
		void generateSingleRequest();
		void processResponses(RequestQueue& requests);
		void processResponse(const ResponseVector responses, const Request::Pointer request,
							ResponseValidator validator);
		bool shouldRemoveWriting(const RequestQueue& requests) const;
		void checkTimeout();

	/* Read Event */
		void readEvent();
		void parseResponse(const std::string& buffer);
		void newResponsePointer();
		void finishResponse();
		bool isFinalResponse(Response::Pointer response);

	/* Write Event */
		void writeEvent();
	
		void waitForServer();

	/* Utility */
		void closeConnection();
		void removeEvent(short int event);
		void addEvent(short int event);
		void setError();
		void setState(State new_state);
		void warning(const std::string& x) const;
		void printRevents() const;

	private:
		static constexpr int CONNECT_MAX_RETRIES = 5;
		static constexpr int POLL_TIMEOUT = 1000;

	private:
		int connfd;
		struct pollfd pfd;
		Settings settings;
		State state;
		Timer timer;
		RequestIterator processing_request;
		RequestStringGenerator string_generator;
		HttpResponseParser response_parser;
		ResponseVector response;
		ResponseQueue responses;
};
