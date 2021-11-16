#pragma once

# include "RequestLineParser.hpp"
# include "HeaderFieldParser.hpp"
# include "ContentParser.hpp"
# include "ChunkedParser.hpp"
# include <string>

class Request;

/*
Parser the buffer, stores information inside of the Request reference
*/
class HttpRequestParser
{
	private:

		enum State
		{
			PARSE_REQUEST_LINE,
			PARSE_HEADER,
			PARSE_CONTENT,
			ERROR,
			COMPLETE
		};

	public:

		int parse(std::string const & buffer, Request& request);

	private:

		RequestLineParser _request_line_parser;
		HeaderFieldParser _header_parser;
		ContentParser _content_parser;
		ChunkedParser _chunked_content_parser;

};
