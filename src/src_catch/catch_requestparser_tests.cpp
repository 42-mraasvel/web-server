#include "parser/RequestParser.hpp"
#include "settings.hpp"
#include "utility/utility.hpp"
#include <sstream>
#include "catch.hpp"

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) sizeof(x) / sizeof(x[0])
#endif

TEST_CASE("Parser: Invalid Request-Lines", "[request_parser]")
{
	// Append CRLF CRLF for header field checking
	// Only testing the request-line parsing
	const std::string inputs[] = {
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
	const std::string inputs[] = {
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

TEST_CASE("Parser: invalid header-fields", "[request-parser]")
{
	const std::string inputs[] = {
		" header-field: header-value",
		"header-field : header-value",
		" hea der-field: header-value",
		"header-field: \r\nheader-value",
	};

	const std::string prefix = "GET / HTTP/1.1" CRLF;

	RequestParser parser;

	for (std::size_t i = 0; i < ARRAY_SIZE(inputs); ++i)
	{
		const std::string header = prefix + inputs[i] + EOHEADER;
		REQUIRE(parser.parseHeader(header) == ERR);
	}

}

TEST_CASE("Parser: basic valid header-fields", "[request-parser]")
{
	/*
	Input format: a, b, c
	a = header-field
	b = result-key
	c = result-value
	*/
	const std::tuple<std::string, std::string, std::string> inputs[] = {
		{"header-field: header-value", "header-field", "header-value"},
		{"header-field:header-value", "header-field", "header-value"},
		{"header-field:     header-value", "header-field", "header-value"},
		{"header-field:\t\t\theader-value", "header-field", "header-value"},
		{"header-field: \theader-value", "header-field", "header-value"},
		{"header-field: header-value    ", "header-field", "header-value"},
		{"header-field: header-value \t\t\t  ", "header-field", "header-value"},
		{"header-field: 1 2 3 4 5", "header-field", "1 2 3 4 5"},
		{"header-field: 1234", "header-field", "1234"},
		{"header-field: 1 2 3 4 5     ", "header-field", "1 2 3 4 5"},
		{"header-field: header-value \t\t\t  ", "header-field", "header-value"},
	};

	const std::string prefix = "GET / HTTP/1.1" CRLF;

	RequestParser parser;

	for (std::size_t i = 0; i < ARRAY_SIZE(inputs); ++i)
	{
		std::string field = std::get<0>(inputs[i]);
		std::string key = std::get<1>(inputs[i]);
		std::string value = std::get<2>(inputs[i]);
		const std::string header = prefix + field + EOHEADER;
		REQUIRE(parser.parseHeader(header) != ERR);
		auto fields = parser.getHeaderFields();
		REQUIRE(fields[key] == value);
	}
}

TEST_CASE("Parser: multiple header-fields", "[request-parser]")
{
	std::string input_fields[] = {
		"0:    0   ",
		"1: 1    ",
		"2: 2",
		"3: 3",
		"4: 4",
		"5: 5",
		"6: 6",
		"7: 7"
	};


	std::string request = "GET / HTTP/1.1" CRLF;
	for (std::size_t i = 0; i < ARRAY_SIZE(input_fields); ++i)
	{
		request += input_fields[i] + CRLF;
	}
	request += CRLF;
	RequestParser parser;
	REQUIRE(parser.parseHeader(request) != ERR);
	for (std::size_t i = 0; i < ARRAY_SIZE(input_fields); ++i)
	{
		std::stringstream ss;
		ss << i;
		std::string key;
		ss >> key;
		REQUIRE(parser.getHeaderFields()[key] == key);
	}
}
