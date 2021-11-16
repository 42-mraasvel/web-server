#include "settings.hpp"
#include "utility/utility.hpp"
#include "parser/ChunkedParser.hpp"
#include <sstream>
#include "catch.hpp"
#include "parser/Request.hpp"
#include "handler/RequestHandler.hpp"

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) sizeof(x) / sizeof(x[0])
#endif

// #define RequestHandler RequestHandler

bool checkNextRequest(RequestHandler& x, Request::RequestStatus expected)
{
	Request* r = x.getNextRequest();
	bool result = r != NULL && r->status == expected;
	delete r;
	return result;
}

/*
	RequestStatus	status;

	MethodType		method;
	std::string		target_resource;
	int				major_version;
	int				minor_version;
	header_field_t	header_fields;
	std::string		message_body;
*/

void printRequest(const std::string& name, const Request& y) {
	std::cout << "-- REQUEST " << name << " -- " << std::endl;
	std::cout << "Status: " << y.status << std::endl;
	std::cout << y.getMethodString() << " " << y.target_resource << " HTTP/" << y.major_version << "." << y.minor_version << std::endl;
	for (auto it = y.header_fields.begin(); it != y.header_fields.end(); ++it) {
		std::cout << it->first << ": " << it->second << std::endl;
	}
	std::cout << y.message_body << std::endl;
}

bool checkNextRequest(RequestHandler& x, const Request& y, bool print = false)
{
	Request* r = x.getNextRequest();
	if (r == NULL) {
		return false;
	}

	if (print) {
		printRequest("INPUT", y);
		printRequest("NEXT_REQUEST", *r);
	}

	bool result = r != NULL &&
		r->status == y.status &&
		r->target_resource == y.target_resource &&
		r->major_version == y.major_version &&
		r->minor_version == y.minor_version &&
		r->header_fields.size() == y.header_fields.size() &&
		std::equal(r->header_fields.begin(), r->header_fields.end(), y.header_fields.begin()) &&
		r->message_body == y.message_body;
	delete r;
	return result;
}

TEST_CASE("Parser: single buffer: many requests", "[request-handler]")
{
	const int TOTAL = 20;
	std::string req =
		"GET / HTTP/1.1" CRLF
		"Content-Length: 13" CRLF
		CRLF
		"HELLO THERE" CRLF;
	std::string buffer;
	for (int i = 0; i < TOTAL; ++i) {
		buffer += req;
	}

	Request example;
	example.status = Request::COMPLETE;
	example.method = GET;
	example.major_version = 1;
	example.minor_version = 1;
	example.target_resource = "/";
	example.message_body = "HELLO THERE\r\n";
	example.header_fields["Content-Length"] = "13";

	RequestHandler parser;
	parser.parse(buffer);
	for (int i = 0; i < TOTAL; ++i)
	{
		REQUIRE(checkNextRequest(parser, example));
	}
	REQUIRE(parser.getNextRequest() == nullptr);
}

TEST_CASE("Parser: partial requests", "[request-handler]")
{
	const int TOTAL = 20;
	std::string req =
		"GET / HTTP/1.1" CRLF
		"Content-Length: 13" CRLF
		CRLF
		"HELLO THERE" CRLF;
	std::string buffer;
	for (int i = 0; i < TOTAL; ++i) {
		buffer += req;
	}

	const std::size_t SEGMENT_SIZE = 10;
	RequestHandler parser;
	for (std::size_t i = 0; i < buffer.size(); i += SEGMENT_SIZE)
	{
		if (parser.parse(buffer.substr(i, SEGMENT_SIZE)) == ERR)
		{
			break;
		}
	}

	Request example;
	example.status = Request::COMPLETE;
	example.method = GET;
	example.major_version = 1;
	example.minor_version = 1;
	example.target_resource = "/";
	example.message_body = "HELLO THERE\r\n";
	example.header_fields["Content-Length"] = "13";

	for (int i = 0; i < TOTAL; ++i) {
		REQUIRE(checkNextRequest(parser, example));
	}

	REQUIRE(parser.getNextRequest() == nullptr);
}

TEST_CASE("Parser: Invalid Request-Lines", "[request-handler]")
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

	RequestHandler parser;
	for (std::size_t i = 0; i < ARRAY_SIZE(inputs); ++i)
	{
		parser.parse(inputs[i] + EOHEADER);
		REQUIRE(checkNextRequest(parser, Request::BAD_REQUEST));
	}
	REQUIRE(parser.getNextRequest() == NULL);
}

// TEST_CASE("Parser: stress testing no header end", "[request-parser]")
// {
// 	RequestHandler parser;
// 	for (std::size_t i = 0; i < 1000000; ++i)
// 	{
// 		parser.parse("a");
// 	}
// }

TEST_CASE("Parser: valid request-lines", "[request-handler]")
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

	RequestHandler parser;

	for (std::size_t i = 0; i < ARRAY_SIZE(inputs); ++i)
	{
		parser.parse(inputs[i] + EOHEADER);
		REQUIRE(checkNextRequest(parser, Request::COMPLETE));
	}
}

TEST_CASE("Parser: invalid header-fields", "[request-handler]")
{
	const std::string inputs[] = {
		" header-field: header-value",
		"header-field : header-value",
		" hea der-field: header-value",
		"header-field: \r\nheader-value",
		"header-field: head\ver-value",
		"header-\nfield: header-value",
		"he\rader-field: header-value",
		"he\tader-field: header-value",
	};

	const std::string prefix = "GET / HTTP/1.1" CRLF;

	RequestHandler parser;

	for (std::size_t i = 0; i < ARRAY_SIZE(inputs); ++i)
	{
		parser.parse(prefix + inputs[i] + EOHEADER);
		REQUIRE(checkNextRequest(parser, Request::BAD_REQUEST));
	}
}

TEST_CASE("Parser: basic valid header-fields", "[request-handler]")
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
	Request example;
	example.status = Request::COMPLETE;
	example.major_version = 1;
	example.minor_version = 1;
	example.method = GET;
	example.target_resource = "/";


	RequestHandler parser;

	for (std::size_t i = 0; i < ARRAY_SIZE(inputs); ++i)
	{
		std::string field = std::get<0>(inputs[i]);
		std::string key = std::get<1>(inputs[i]);
		std::string value = std::get<2>(inputs[i]);
		parser.parse(prefix + field + EOHEADER);
		example.header_fields.clear();
		example.header_fields[key] = value;

		REQUIRE(checkNextRequest(parser, example));
	}
	REQUIRE(parser.getNextRequest() == NULL);
}

TEST_CASE("Parser: multiple header-fields", "[request-handler]")
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

	Request example;
	example.method = GET;
	example.target_resource = "/";
	example.major_version = 1;
	example.minor_version = 1;
	example.status = Request::COMPLETE;

	std::string request = "GET / HTTP/1.1" CRLF;
	for (std::size_t i = 0; i < ARRAY_SIZE(input_fields); ++i)
	{
		request += input_fields[i] + CRLF;

		std::stringstream ss;
		ss << i;
		example.header_fields[ss.str()] = ss.str();
	}
	request += CRLF;
	RequestHandler parser;
	parser.parse(request);

	REQUIRE(checkNextRequest(parser, example));
	REQUIRE(parser.getNextRequest() == NULL);
}

TEST_CASE("parser: chunked", "[request-handler]")
{
	std::string input = 
		"GET / HTTP/1.1" CRLF
		"Host: 127.0.0.1:80" CRLF
		"Content-Type: text/plain" CRLF
		"Transfer-Encoding: Chunked"
		EOHEADER
		"7 ; comment" CRLF
		"Mozilla" CRLF
		"9    ;;  " CRLF
		"Developer" CRLF
		"7" CRLF
		"Network" CRLF
		"0" CRLF
		"Trailer: Value" CRLF
		"Trailer1: Value1" CRLF
		"Trailer2: Value2" CRLF
		CRLF;

	RequestHandler parser;

	std::size_t index = 0;

	for (std::size_t i = 0; i < input.size(); ++i)
	{
		parser.parse(input.substr(i, 1));
	}

	// parser.parse(input);

	Request example;
	example.status = Request::COMPLETE;
	example.major_version = 1;
	example.method = GET;
	example.target_resource = "/";
	example.minor_version = 1;
	example.header_fields["Host"] = "127.0.0.1:80";
	example.header_fields["Content-Type"] = "text/plain";
	example.header_fields["Transfer-Encoding"] = "Chunked";
	example.header_fields["Trailer"] = "Value";
	example.header_fields["Trailer1"] = "Value1";
	example.header_fields["Trailer2"] = "Value2";
	example.message_body = "MozillaDeveloperNetwork";

	REQUIRE(checkNextRequest(parser, example));
}

TEST_CASE("parser: chunked invalid", "[request-handler]")
{
	std::string inputs[] = {
		"Transfer-Encoding: chunked, chunked" EOHEADER "7" CRLF "Mozilla" CRLF "0" EOHEADER,
		"Transfer-Encoding: chunked" EOHEADER "1247192348912341247123" CRLF "Mozilla" CRLF "0" EOHEADER,
		"Transfer-Encoding: chunked" EOHEADER "Z" CRLF "0" EOHEADER
	};

	std::string prefix = "GET / HTTP/1.1" CRLF;

	RequestHandler parser;
	for (std::size_t i = 0; i < ARRAY_SIZE(inputs); ++i) {
		parser.parse(prefix + inputs[i]);
		Request* r = parser.getNextRequest();
		REQUIRE(r != NULL);
		REQUIRE(r->status == Request::BAD_REQUEST);
		delete r;
	}
}
