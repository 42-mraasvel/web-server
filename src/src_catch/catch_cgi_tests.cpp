#include "catch.hpp"
#include "settings.hpp"
#include "utility/utility.hpp"
#include "cgi/CgiHandler.hpp"
#include "config/ConfigLocation.hpp"
#include <poll.h>

	std::string											_path;
	std::string											_root;
	std::pair<int, std::string>							_return;
	std::vector<std::string>							_index;
	std::vector<std::string>							_allowed_methods;
	std::vector<std::pair<std::string, std::string> >	_cgi;
	bool												_autoindex_status;
	location_flag										_location_flag;

static LocationBlock* generateCgiBlock() {
	LocationBlock* x = new LocationBlock;

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
	r.config_info.resolved_location = generateCgiBlock();
	r.request_target = "/x.py";
	REQUIRE(CgiHandler::isCgi(r) == true);
	r.request_target = "/a/b/x.py";
	REQUIRE(CgiHandler::isCgi(r) == true);
	r.request_target = "/a/b/x.py/a/b/x";
	REQUIRE(CgiHandler::isCgi(r) == true);
	r.request_target = "/1234x.py";
	REQUIRE(CgiHandler::isCgi(r) == true);
	r.request_target = "/asdf/asdf/ads/fadsfx.py";
	REQUIRE(CgiHandler::isCgi(r) == true);
	r.request_target = "/asdf/asdf/ads/.py";
	REQUIRE(CgiHandler::isCgi(r) == true);
	r.request_target = "/.py";
	REQUIRE(CgiHandler::isCgi(r) == true);
	r.request_target = "/.py/";
	REQUIRE(CgiHandler::isCgi(r) == true);
	r.request_target = "/.py/a";
	REQUIRE(CgiHandler::isCgi(r) == true);

/* False Tests */
	r.request_target = "/";
	REQUIRE(CgiHandler::isCgi(r) == false);
	r.request_target = "/x.pyz";
	REQUIRE(CgiHandler::isCgi(r) == false);
	r.request_target = "/x.pyz/asdf";
	REQUIRE(CgiHandler::isCgi(r) == false);
	r.request_target = "/x.pyz/py./py";
	REQUIRE(CgiHandler::isCgi(r) == false);
	r.request_target = "/x.pyz/asdf/.p/";
	REQUIRE(CgiHandler::isCgi(r) == false);
	r.request_target = "/x.p";
	REQUIRE(CgiHandler::isCgi(r) == false);
	r.request_target = "/x.pys/";
	REQUIRE(CgiHandler::isCgi(r) == false);
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
