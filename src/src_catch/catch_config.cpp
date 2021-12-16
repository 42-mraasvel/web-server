#include "catch.hpp"
#include "config/Config.hpp"
#include "settings.hpp"
#include <iostream>

std::vector<Config> conf_vec

TEST_CASE("Basic Config Parsing", "[config]")
{
    typedef Config::address_map::iterator iterator;
    Config conf("src/src_normal/config/resources/default.conf");
    REQUIRE(conf.parser() == OK);
}
