#pragma once

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
			FINISHED
		};

	public:

		ChunkedParser();

	/*
	Parses the next buffer and appends it to body
	*/
		int parse(std::string const & buffer, std::size_t & index, std::string & body);

	/*
	Sends body as a chunked message
	Returns the total characters that extracted from body and sent
	*/
		static ssize_t send(int fd, std::string const & body);

	private:

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
		std::size_t _index;
		std::size_t _chunk_size;
		std::string _buffer;
		std::string _leftover;
		bool _received_cr;
};
