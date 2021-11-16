#pragma once

# include "RequestLineParser.hpp"
# include "HeaderFieldParser.hpp"
# include "ContentParser.hpp"
# include "ChunkedParser.hpp"
# include <string>

struct Request;

/*
Parser the buffer, stores information inside of the Request reference
Extracts the next request from the buffer,
sets index to buffer.size() if incomplete request
*/
class HttpRequestParser
{
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

		HttpRequestParser();

		int parse(std::string const & buffer, Request& request);

		int getStatusCode() const;
		bool isError() const;
		bool isComplete() const;
		void reset();

	private:

		int setComplete();
		int setError(int code);

		void parseRequestLine(std::string const & buffer,
			std::size_t & index, Request & request);
		void parseHeader(std::string const & buffer,
			std::size_t & index, Request & request);
		void parseContent(std::string const & buffer,
			std::size_t & index, Request & request);
		void parseChunked(std::string const & buffer,
			std::size_t & index, Request & request);


	private:

		State _state;
		int _status_code;

		RequestLineParser _request_line_parser;
		HeaderFieldParser _header_parser;
		ContentParser _content_parser;
		ChunkedParser _chunked_content_parser;
};
