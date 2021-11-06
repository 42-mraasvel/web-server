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
	r.target_resource = "/x.py";
	REQUIRE(cgi_handler.isCgi(&r) == true);
	r.target_resource = "/a/b/x.py";
	REQUIRE(cgi_handler.isCgi(&r) == true);
	r.target_resource = "/a/b/x.py/a/b/x";
	REQUIRE(cgi_handler.isCgi(&r) == true);
	r.target_resource = "/1234x.py";
	REQUIRE(cgi_handler.isCgi(&r) == true);
	r.target_resource = "/asdf/asdf/ads/fadsfx.py";
	REQUIRE(cgi_handler.isCgi(&r) == true);
	r.target_resource = "/asdf/asdf/ads/.py";
	REQUIRE(cgi_handler.isCgi(&r) == true);
	r.target_resource = "/.py";
	REQUIRE(cgi_handler.isCgi(&r) == true);
	r.target_resource = "/.py/";
	REQUIRE(cgi_handler.isCgi(&r) == true);
	r.target_resource = "/.py/a";
	REQUIRE(cgi_handler.isCgi(&r) == true);

/* False Tests */
	r.target_resource = "/";
	REQUIRE(cgi_handler.isCgi(&r) == false);
	r.target_resource = "/x.pyz";
	REQUIRE(cgi_handler.isCgi(&r) == false);
	r.target_resource = "/x.pyz/asdf";
	REQUIRE(cgi_handler.isCgi(&r) == false);
	r.target_resource = "/x.pyz/py./py";
	REQUIRE(cgi_handler.isCgi(&r) == false);
	r.target_resource = "/x.pyz/asdf/.p/";
	REQUIRE(cgi_handler.isCgi(&r) == false);
	r.target_resource = "/x.p";
	REQUIRE(cgi_handler.isCgi(&r) == false);
	r.target_resource = "/x.pys/";
	REQUIRE(cgi_handler.isCgi(&r) == false);
}

TEST_CASE("Cgi Generate Meta-variables") {
	// Request r;
	// CgiHandler cgi;

	// FdTable fd_table;

	// r.target_resource = "/cgi-bin/cgi.py/path/info";
	// r.query = "abcde=sa=d%20";
	// r.method = GET;
	// r.minor_version = 1;
	// r.major_version = 1;
	// r.message_body = "1234";
	// r.header_fields["Content-Type"] = "plain/text";
	// REQUIRE(cgi.isCgi(&r) == true);
	// cgi.execute(&r, fd_table);
}
