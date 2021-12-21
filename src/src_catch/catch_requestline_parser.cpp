#include "catch.hpp"
#include "parser/RequestLineParser.hpp"
#include "request/Request.hpp"
#include "settings.hpp"

TEST_CASE("Request Line Basic Tests", "[parser]")
{
	std::string input =
		"GET /index.html?a=u&x=y# HTTP/1.1" CRLF
		"bla: bla"
		CRLF;

	RequestLineParser parser;
	RequestLineParser parser2;

	Request r;
	for (std::size_t i = 0; i < input.size(); ++i) {
		std::size_t index = 0;
		parser.parse(input.substr(i, 1), index, r);
		if (parser.isComplete()) {
			break;
		}
	}
	Request r2;
	std::size_t index = 0;
	parser2.parse(input, index, r2);
}
