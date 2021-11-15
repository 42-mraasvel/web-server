#include "catch.hpp"
#include "cgi/CgiResponseParser.hpp"
#include "settings.hpp"

TEST_CASE("Cgi Response Parser", "[cgi]") {
	std::string header =
		"Content-Length: 151" CRLF
		"Content-Type: text/html" CRLF
		CRLF;
	std::string content =
		"<html>\n"
		"	<head>\n"
		"		<title>Hello World - First CGI Program</title>\n"
		"	</head>\n"
		"	<body>\n"
		"		<h2>Hello World! This is my first CGI program</h2>\n"
		"	</body>\n"
		"</html>\n";

	CgiResponseParser p;

	p.parse(header + content);
	REQUIRE(p.getState() == CgiResponseParser::COMPLETE);
	REQUIRE(p.getContent() == content);
}
