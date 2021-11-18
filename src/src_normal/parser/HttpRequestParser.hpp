#pragma once

# include "RequestLineParser.hpp"
# include "HeaderFieldParser.hpp"
# include "ContentParser.hpp"
# include "ChunkedParser.hpp"
# include "handler/RequestHeaderProcessor.hpp"
# include <string>

struct Request;

/*
Parser the buffer, stores information inside of the Request reference
Extracts the next request from the buffer,
sets index to buffer.size() if incomplete request
*/
class HttpRequestParser
{
	public:
		typedef	ConfigResolver::MapType		MapType;
		typedef	ConfigResolver::AddressType	AddressType;

	private:
		enum State
		{
			PARSE_REQUEST_LINE,
			PARSE_HEADER,
			PARSE_CONTENT,
			PARSE_CHUNKED,
			ERROR,
			COMPLETE
		};

	public:
		HttpRequestParser(AddressType address, MapType* config_map);

		int parse(std::string const & buffer, std::size_t & index, Request& request);

		int getStatusCode() const;
		bool isError() const;
		bool isComplete() const;
		void reset();

	private:

		int setComplete();
		int setError(int code);
		void setState(State new_state);

	/* Main Parsing */
		void parseRequestLine(std::string const & buffer,
			std::size_t & index, Request & request);
		void parseHeader(std::string const & buffer,
			std::size_t & index, Request & request);
		void parseContent(std::string const & buffer,
			std::size_t & index, Request & request);
		void parseChunked(std::string const & buffer,
			std::size_t & index, Request & request);

	/* HeaderFields */
		int processRequestHeader(Request & request);
		int checkContentType(HeaderField const & header);
		int parseContentLength(std::string const & value);
		int parseTransferEncoding(std::string const & value);

	private:

		State _state;
		int _status_code;

		RequestLineParser _request_line_parser;
		HeaderFieldParser _header_parser;
		ContentParser _content_parser;
		ChunkedParser _chunked_content_parser;

		RequestHeaderProcessor _header_processor;
};
