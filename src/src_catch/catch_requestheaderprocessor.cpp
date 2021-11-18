#include "catch.hpp"
#include "handler/RequestHeaderProcessor.hpp"
#include "handler/RequestHandler.hpp"
#include "settings.hpp"
#include <vector>

TEST_CASE("Valid RHP", "[request-header-processor]")
{
}

TEST_CASE("Close Connection", "[request-header-processor-tmp]")
{
	RequestHandler handler;

	std::vector<std::string> inputs = {
		"GET / HTTP/1.1" EOHEADER,
		"GET / HTTP/1.0" EOHEADER,
		"GET / HTTP/1.1" CRLF
			"host: localhost" CRLF
			"connection: close" EOHEADER,
	};


	for (const std::string& input : inputs)
	{
		handler.parse(input);
		Request* r = handler.getNextRequest();
		REQUIRE(r != NULL);
		REQUIRE(r->close_connection == true);
		delete r;
	}

}

TEST_CASE("Invalid HeaderFields", "[request-header-processor]")
{
	std::vector<std::pair<std::string, int> > inputs = {
		{"Content-Coding: x", StatusCode::UNSUPPORTED_MEDIA_TYPE},
		{"Content-Coding: ", StatusCode::BAD_REQUEST},
		{"Transfer-Encoding: chunk", StatusCode::NOT_IMPLEMENTED},
		{"Transfer-Encoding: chunked, chunked", StatusCode::NOT_IMPLEMENTED},
		{"Content-Length: 123" CRLF "Content-Length: 1", StatusCode::BAD_REQUEST},
		{"Host: localhost" CRLF "Host: localhost", StatusCode::BAD_REQUEST},
		{"Transfer-Encoding: chunked" CRLF "Transfer-Encoding: chunked", StatusCode::BAD_REQUEST},
	};

	std::string prefix = "GET / HTTP/1.0" CRLF;
	RequestHandler handler;

	for (const std::pair<std::string, int>& field : inputs) {
		std::string input = prefix + field.first + EOHEADER;
		handler.parse(input);
		Request* r = handler.getNextRequest();
		REQUIRE(r);
		REQUIRE(r->status == Request::BAD_REQUEST);
		REQUIRE(r->status_code == field.second);
	}
}
