#include "catch.hpp"

TEST_CASE("Example Catch Test Name", "[example_tag]") {
	std::string x;

	REQUIRE(x.size() == 0);
	x = "123";
	REQUIRE(x.size() == 3);
}
