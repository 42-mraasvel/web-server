#pragma once

# include <string>
# include <sys/types.h> // ssize_t

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

		int parseChunkSize(std::string const & buffer);
		int parseChunkData(std::string const & buffer, std::string & body);
		int parseTrailer(std::string const & buffer);

		int parseEndLine(std::string const & buffer);
		bool hasCRLF(std::string const & buffer);

	/* Debugging */

		void print(const std::string& buffer) const;
		std::string getStateString() const;

	private:

		State _state;
		State _next_state;
		std::size_t _index;
		std::size_t _chunk_size;
		std::string _buffer;
		bool _received_cr;
};
