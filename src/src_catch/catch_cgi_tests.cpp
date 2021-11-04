#include "catch.hpp"
#include "settings.hpp"
#include "utility/utility.hpp"
#include "cgi/CgiHandler.hpp"

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
