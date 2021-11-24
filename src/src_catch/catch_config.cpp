#include "catch.hpp"
#include "config/Config.hpp"
#include <iostream>

TEST_CASE("Basic Config Parsing", "[config]")
{
    typedef Config::address_map::iterator iterator;
    Config conf("src/src_normal/config/resources/default.conf");
    Config::address_map address_map = conf.getAddressMap();
    iterator it = address_map.begin();
}







