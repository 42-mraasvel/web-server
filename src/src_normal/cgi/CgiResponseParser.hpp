#pragma once

#include "parser/HeaderFieldParser.hpp"
#include "parser/ContentParser.hpp"

class CgiResponseParser
{
	public:
		enum State
		{
			PARSE_HEADER,
			PARSE_CONTENT,
			COMPLETE,
			ERROR
		};

	public:

		CgiResponseParser();

		int parse(std::string const & buffer);
		HeaderField& getHeader();
		std::string& getContent();
		void reset();

		State getState() const;
		int getStatusCode() const;

	private:

		void parseHeader(std::string const & buffer, std::size_t & index);
		void parseContent(std::string const & buffer, std::size_t & index);
		int setError(int status);
		void setState(State state);
		void setContentParsing();

	private:
		HeaderFieldParser _header_parser;
		ContentParser _content_parser;
		State _state;
		int _status_code;
};
