#pragma once

# include <string>

struct Request;

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
		void reset();
	
	private:
		bool hasEndLine(std::string const & buffer, std::size_t index);
		int appendLeftover(std::string const & buffer, std::size_t & index,
						std::size_t len);
		int setError(int code);
		int setComplete();

	/* Actual Parsing */
	
		int		parseRequestLine(Request & request);
		int		parseSpace();
		int		parseMethod(Request & request);
		int		parseTargetResource(Request & request);
		int		skipAbsolutePath();
		int		skipQuery();
		int		parseVersion(Request & request);
		int		parseMajorVersion(Request & request);
		int		parseMinorVersion(Request & request);
		int		parseEndLine() const;


	private:

		State _state;
		std::string _leftover;
		std::size_t _index;
		int _status_code;
};
