#include "catch.hpp"
#include "handler/RequestHeaderProcessor.hpp"
#include "handler/RequestHandler.hpp"
#include "settings.hpp"

TEST_CASE("Valid RHP", "[request-header-processor]")
{
}

TEST_CASE("Close Connection", "[request-header-processor]")
{
	RequestHandler handler;

	std::string input =
		"GET / HTTP/1.1" CRLF
		CRLF;

	handler.parse(input);
	Request* r = handler.getNextRequest();
	REQUIRE(r != NULL);
	REQUIRE(r->close_connection == true);
}

