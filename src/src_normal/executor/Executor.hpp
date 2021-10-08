#pragma once
#include "parser/RequestParser.hpp"

//TODO: create response parser
class Executor
{
	public:
		int execute(RequestParser const & parser);
		std::string const & getResponse() const;

	private:
		int methodGet();
		int methodPost();
		int methodDelete();
		int genericResponse();

	private:
		std::string _response;

};

