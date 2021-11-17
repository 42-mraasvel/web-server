#pragma once

# include "HeaderFieldParser.hpp"
# include "ContentParser.hpp"
# include <string>
# include <sys/types.h> // ssize_t
# include <vector>

struct Request;

class ChunkedParser
{
	private:

	/*
	State is used to keep track of where in the finite state machine the parser left off
	Initial state is SIZE
	*/
		enum State
		{
			SIZE,
			DATA,
			TRAILER,
			ENDLINE,
			DISCARD_LINE,
			COMPLETE,
			ERROR
		};

	public:

		ChunkedParser();

	/*
	Parses the next buffer and appends it to body
	*/
		int parse(std::string const & buffer, std::size_t & index, Request & request);

		void setMaxSize(std::size_t max);
		bool isParsing() const;
		bool isComplete() const;
		bool isError() const;
		int getStatusCode() const;
		void reset();

	private:

		int setComplete();
		int setError();
		int setError(int code);
		int addHeaderFields(Request & request);

		int parseSize(std::string const & buffer, std::size_t & index, Request & request);
		int parseData(std::string const & buffer, std::size_t & index, Request & request);
		int parseTrailer(std::string const & buffer, std::size_t & index, Request & request);
		int parseEndLine(std::string const & buffer, std::size_t & index, Request & request);
		int parseDiscardLine(std::string const & buffer, std::size_t & index, Request & request);

		typedef int (ChunkedParser::*StateParserType)(std::string const & buffer,
														std::size_t & index,
														Request & request);
		typedef std::vector<StateParserType> StateDispatchTableType;

		static StateDispatchTableType createStateDispatch();

	/* Debugging */

		void print(const std::string& buffer, std::size_t index) const;
		std::string getStateString(State state) const;

	private:

		State _state;
		State _next_state;
		std::size_t _max_size;
		std::size_t _chunk_size;
		std::string _leftover;
		int _status_code;

		ContentParser _content_parser;
		HeaderFieldParser _header_parser;
};
