#pragma once

# include <string>
# include "utility/status_codes.hpp"

/*
What does this parser parse:
	- Raw content (content-length)

ContentLength:
	If set to std::numeric_limits<std::size_t>::max(),
		will be considered unknown contentLength, AKA parse as much as possible

Errors:
	- Payload too large (exceeds max_body_size)
*/
class ContentParser
{
	private:
		enum State
		{
			PARSING,
			COMPLETE,
			ERROR
		};

	public:
		ContentParser();

		int parse(std::string const & buffer, std::size_t& index);

		std::string& getContent();
		void reset();

		bool isError() const;
		bool isComplete() const;
		int getStatusCode() const;

		void setContentLength(std::size_t length);
		void setMaxSize(std::size_t max);

	private:
		int parseRawData(std::string const & buffer, std::size_t& index);
		int setError(StatusCode::Code code);
		void setComplete();

	private:
		std::string _content;
		std::size_t	_content_length;
		std::size_t _max_size;
		State _state;
		int _status_code;
};
