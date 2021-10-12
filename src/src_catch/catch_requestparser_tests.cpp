#include "parser/RequestParser.hpp"
#include "settings.hpp"
#include "catch.hpp"

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) sizeof(x) / sizeof(x[0])
#endif

TEST_CASE("Parser: Invalid Request-Lines", "[request_parser]")
{
	// Append CRLF CRLF for header field checking
	// Only testing the request-line parsing
	static const std::string inputs[] = {
		" GET / HTTP/1.1",
		"GET / HTTP/1.1 ",
		"GET /  HTTP/1.1",
		"GET  / HTTP/1.1",
		"GET / HTTP /1.1",
		"GET / HTTP/1.1000",
		"GET / HTTP/0.1",
		"GET / HTTP/01.1",
		" / HTTP/1.1",
		"GET / ",
		"GET  HTTP/1.1",
		"GET 1234/ HTTP/1.1",
		"GET / aHTTP/1.1",
		"GET / HTTP/a1.1",
		"GET / HTTP/11.",
		"GET / HTTP/1.",
		": / HTTP/1.1",
		"POST /\t HTTP/1.1",
		"POST / HTTP/.1",
	};

	RequestParser parser;

	for (std::size_t i = 0; i < ARRAY_SIZE(inputs); ++i)
	{
		REQUIRE(parser.parseHeader(inputs[i] + EOHEADER) == ERR);
	}

	// Tests without EOHEADER
	REQUIRE(parser.parseHeader("GET / HTTP/1.1" CRLF) == ERR);
	REQUIRE(parser.parseHeader("GET / HTTP/1.1") == ERR);
}

TEST_CASE("Parser: valid request-lines", "[request-parser]")
{
	static const std::string inputs[] = {
		"GET / HTTP/1.1",
		"GET / HTTP/11234123412341234123412341234.1",
		"GET / HTTP/1.999",
		"GET / HTTP/10.1",
		"11Gabd3 / HTTP/1.1",
		"GET /1234/1234/?a%ad HTTP/1.1",
		"AOISDJOIASJDOIAJSDIOJASD / HTTP/1.1",
		"POSTERS /11111/1/1/1/1/2/3/4/5/6/7198274981273 HTTP/1.123",
	};

	RequestParser parser;

	for (std::size_t i = 0; i < ARRAY_SIZE(inputs); ++i)
	{
		REQUIRE(parser.parseHeader(inputs[i] + EOHEADER) != ERR);
	}

	REQUIRE(parser.parseHeader("GET /abc/def HTTP/1.1\r\n\r\n") != ERR);
	REQUIRE(parser.getMethod() == RequestParser::GET);
	REQUIRE(parser.getHttpVersion().major == 1);
	REQUIRE(parser.getHttpVersion().minor == 1);
	REQUIRE(parser.getTargetResource() == "/abc/def");
}
