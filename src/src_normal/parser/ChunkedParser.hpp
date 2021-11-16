#pragma once

# include "HeaderFieldParser.hpp"
# include "ContentParser.hpp"
# include <string>
# include <sys/types.h> // ssize_t
# include <vector>

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
			ERROR,
			COMPLETE,
			FINISHED
		};

	public:

		ChunkedParser();

	/*
	Parses the next buffer and appends it to body
	*/
		int parse(std::string const & buffer, std::size_t & index, std::string & body);
		bool finished() const;

		void setMaxSize(std::size_t max);
		bool isParsing() const;
		bool isComplete() const;
		bool isError() const;
		void reset();

	private:

		int setComplete();
		int setError();

		int parseSize(std::string const & buffer, std::size_t & index, std::string & body);
		int parseData(std::string const & buffer, std::size_t & index, std::string & body);
		int parseTrailer(std::string const & buffer, std::size_t & index, std::string & body);
		int parseEndLine(std::string const & buffer, std::size_t & index, std::string & body);
		int parseDiscardLine(std::string const & buffer, std::size_t & index, std::string & body);

		bool hasCRLF(std::string const & buffer, std::size_t index);

		typedef int (ChunkedParser::*StateParserType)(std::string const & buffer,
														std::size_t & index,
														std::string & body);
		typedef std::vector<StateParserType> StateDispatchTableType;

		static StateDispatchTableType createStateDispatch();

		typedef bool (*IsFunctionType)(char x);
		void skip(std::string const & buffer, std::size_t & index, IsFunctionType callback);


	/* Debugging */

		void print(const std::string& buffer, std::size_t index) const;
		std::string getStateString(State state) const;

	private:

		State _state;
		State _next_state;
		std::size_t _max_size;
		std::size_t _chunk_size;
		std::string _leftover;

		ContentParser _content_parser;
		HeaderFieldParser _header_parser;
};
