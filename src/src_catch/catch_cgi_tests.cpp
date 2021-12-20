#include "catch.hpp"
#include "settings.hpp"
#include "utility/utility.hpp"
#include "handler/CgiHandler.hpp"
#include "config/ConfigLocation.hpp"
#include <poll.h>

static SmartPointer<LocationBlock> generateCgiBlock() {
	SmartPointer<LocationBlock> x(new LocationBlock);

	x->_path = "/";
	x->_root = "./page_sample";
	x->_index = {"index.html", "index.py"};
	x->_allowed_methods = {"GET", "POST", "DELETE"};
#ifdef __linux__
	x->_cgi = { {".py", "/usr/bin/python3"}};
#else
	x->_cgi = { {".py", "/Users/mraasvel/.brew/bin/python3"}};
#endif /* __linux__ */
	x->_autoindex_status = true;
	x->_location_flag = NONE;
	return x;
}

/* Note: Configuration not integrated yet */
TEST_CASE("IsCgi tests", "[cgi]")
{
/* True Tests */
	Request r;
	r.config_info.result = ConfigInfo::LOCATION_RESOLVED;
	r.config_info.resolved_location = generateCgiBlock();
	r.config_info.resolved_target = "/x.py";
	REQUIRE(CgiHandler::isCgi(r) == true);
	r.config_info.resolved_target = "/a/b/x.py";
	REQUIRE(CgiHandler::isCgi(r) == true);
	r.config_info.resolved_target = "/1234x.py";
	REQUIRE(CgiHandler::isCgi(r) == true);
	r.config_info.resolved_target = "/asdf/asdf/ads/fadsfx.py";
	REQUIRE(CgiHandler::isCgi(r) == true);
	r.config_info.resolved_target = "/asdf/asdf/ads/.py";
	REQUIRE(CgiHandler::isCgi(r) == true);
	r.config_info.resolved_target = "/.py";

/* False Tests */
	r.config_info.resolved_target = "/";
	REQUIRE(CgiHandler::isCgi(r) == false);
	r.config_info.resolved_target = "/x.pyz";
	REQUIRE(CgiHandler::isCgi(r) == false);
	r.config_info.resolved_target = "/x.pyz/asdf";
	REQUIRE(CgiHandler::isCgi(r) == false);
	r.config_info.resolved_target = "/x.pyz/py./py";
	REQUIRE(CgiHandler::isCgi(r) == false);
	r.config_info.resolved_target = "/x.pyz/asdf/.p/";
	REQUIRE(CgiHandler::isCgi(r) == false);
	r.config_info.resolved_target = "/x.p";
	REQUIRE(CgiHandler::isCgi(r) == false);
	r.config_info.resolved_target = "/x.pys/";
	REQUIRE(CgiHandler::isCgi(r) == false);
}

TEST_CASE("Cgi Generate Meta-variables") {
	// Request r;
	// CgiHandler cgi;

	// FdTable fd_table;

	// r.config_info.resolved_target = "/cgi-bin/cgi.py/path/info";
	// r.query = "abcde=sa=d%20";
	// r.method = GET;
	// r.minor_version = 1;
	// r.major_version = 1;
	// r.message_body = "1234";
	// r.header_fields["Content-Type"] = "plain/text";
	// REQUIRE(cgi.isCgi(r) == true);
	// cgi.execute(r, fd_table);
}
