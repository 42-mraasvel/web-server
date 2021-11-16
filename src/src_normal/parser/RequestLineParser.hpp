#pragma once

# include <string>

class Request;

class RequestLineParser
{
	private:
		enum State
		{
			PARSE,
			COMPLETE,
			ERROR
		};

	public:

		RequestLineParser();

		int parse(std::string const & buffer, std::size_t & index, Request & request);

		int getStatusCode() const;
		bool isComplete() const;
		bool isError() const;
	
	private:

		bool hasEndLine(std::string const & buffer, std::size_t index);
		int appendLeftover(std::string const & buffer, std::size_t & index);
		int setError(int code);

	private:

		State _state;
		std::string _leftover;
		int _status_code;
};
