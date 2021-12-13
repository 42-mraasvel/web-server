#pragma once

#include "StatusLineParser.hpp"
#include "ContentParser.hpp"
#include "ChunkedParser.hpp"
#include "HeaderFieldParser.hpp"
#include "DataStructures/Response.hpp"
#include <string>

class HttpResponseParser {
	private:
		enum class State {
			PARSE_STATUS_LINE,
			PARSE_HEADER,
			PARSE_CONTENT,
			PARSE_CHUNKED,
			ERROR,
			COMPLETE
		};

	public:
		HttpResponseParser();

		int parse(const std::string& buffer, std::size_t& index, Response& dest);

		bool isError() const;
		bool isComplete() const;
		void reset();
	
	private:
		void setState(State new_state);
		void setError();

	/* Main Parsing */
		void parseStatusLine(const std::string& buffer,
			std::size_t & index, Response & response);
		void parseHeader(const std::string& buffer,
			std::size_t & index, Response & response);
		void parseContent(const std::string& buffer,
			std::size_t & index, Response & response);
		void parseChunked(const std::string& buffer,
			std::size_t & index, Response & response);

	/* Header Processing */
		void processHeader(const HeaderField& header);
		void checkContent(const HeaderField& header);
		void checkContentLength(const std::string& value);
		void checkTransferEncoding(const std::string& value);

	private:
		State state;
		StatusLineParser status_line_parser;
		HeaderFieldParser header_field_parser;
		ContentParser content_parser;
		ChunkedParser chunked_content_parser;
};
