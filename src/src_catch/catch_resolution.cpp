#include "catch.hpp"
#include "handler/RequestHandler.hpp"
#include "settings.hpp"
#include "tmp/create_address_map.hpp"

TEST_CASE("Basic Resolution", "[resolution]")
{
	std::string input =
		"GET /x HTTP/1.1" CRLF
		"Host: localhost:8080" CRLF
		"Content-Length: 10" CRLF
		CRLF
		"1234567890";
	
	ConfigResolver::MapType* x = testing::createAddressMap();
	RequestHandler handler(testing::createAddress(), x);

	handler.parse(input);

	Request* r = handler.getNextRequest();

	REQUIRE(r);
}
