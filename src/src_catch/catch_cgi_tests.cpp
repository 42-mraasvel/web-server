#include "catch.hpp"
#include "settings.hpp"
#include "utility/utility.hpp"
#include "cgi/CgiHandler.hpp"
#include <poll.h>

/* Note: Configuration not integrated yet */
TEST_CASE("IsCgi tests", "[cgi]")
{
	CgiHandler cgi_handler;

/* True Tests */
	Request r;
	r.request_target = "/x.py";
	REQUIRE(cgi_handler.isCgi(r) == true);
	r.request_target = "/a/b/x.py";
	REQUIRE(cgi_handler.isCgi(r) == true);
	r.request_target = "/a/b/x.py/a/b/x";
	REQUIRE(cgi_handler.isCgi(r) == true);
	r.request_target = "/1234x.py";
	REQUIRE(cgi_handler.isCgi(r) == true);
	r.request_target = "/asdf/asdf/ads/fadsfx.py";
	REQUIRE(cgi_handler.isCgi(r) == true);
	r.request_target = "/asdf/asdf/ads/.py";
	REQUIRE(cgi_handler.isCgi(r) == true);
	r.request_target = "/.py";
	REQUIRE(cgi_handler.isCgi(r) == true);
	r.request_target = "/.py/";
	REQUIRE(cgi_handler.isCgi(r) == true);
	r.request_target = "/.py/a";
	REQUIRE(cgi_handler.isCgi(r) == true);

/* False Tests */
	r.request_target = "/";
	REQUIRE(cgi_handler.isCgi(r) == false);
	r.request_target = "/x.pyz";
	REQUIRE(cgi_handler.isCgi(r) == false);
	r.request_target = "/x.pyz/asdf";
	REQUIRE(cgi_handler.isCgi(r) == false);
	r.request_target = "/x.pyz/py./py";
	REQUIRE(cgi_handler.isCgi(r) == false);
	r.request_target = "/x.pyz/asdf/.p/";
	REQUIRE(cgi_handler.isCgi(r) == false);
	r.request_target = "/x.p";
	REQUIRE(cgi_handler.isCgi(r) == false);
	r.request_target = "/x.pys/";
	REQUIRE(cgi_handler.isCgi(r) == false);
}

TEST_CASE("Cgi Generate Meta-variables") {
	// Request r;
	// CgiHandler cgi;

	// FdTable fd_table;

	// r.request_target = "/cgi-bin/cgi.py/path/info";
	// r.query = "abcde=sa=d%20";
	// r.method = GET;
	// r.minor_version = 1;
	// r.major_version = 1;
	// r.message_body = "1234";
	// r.header_fields["Content-Type"] = "plain/text";
	// REQUIRE(cgi.isCgi(r) == true);
	// cgi.execute(r, fd_table);
}
