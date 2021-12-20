#pragma once

#include "DataStructures/Response.hpp"
#include <string>

class StatusLineParser {
	private:
		enum class State {
			PARSING,
			COMPLETE,
			ERROR
		};
	
	public:
		StatusLineParser();

		int parse(const std::string& buffer, std::size_t& index, Response& dest);

		bool isComplete() const;
		bool isError() const;
		void reset();
	
	private:
		int appendLeftover(const std::string& buffer, std::size_t& index, std::size_t len);
		int parseStatusLine(Response& dest);

		typedef int (StatusLineParser::*ParseFunction)(Response& dest, std::size_t& index);
		int parseVersion(Response& dest, std::size_t& index);
		int parseMajorVersion(Response& dest, std::size_t& index);
		int parseMinorVersion(Response& dest, std::size_t& index);

		int parseChar(std::size_t& index, char c);
		int parseSpace(Response& dest, std::size_t& index);

		int parseStatusCode(Response& dest, std::size_t& index);
		int parseReasonPhrase(Response& dest, std::size_t& index);


		int setError();
		void setState(State new_state);
	private:
		State state;
		std::string leftover;
};
