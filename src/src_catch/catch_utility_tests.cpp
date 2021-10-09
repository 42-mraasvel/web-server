#include "utility/utility.hpp"
#include "catch.hpp"

TEST_CASE("splitString: single delim", "[utility]")
{
	const char delim = ' ';

	const std::vector<std::string> inputs = {
		"",
		"1",
		" 1",
		"1 ",
		"1 2",
		" 1 2",
		"1 2 ",
		"1             2 3 4  5 6 7 8 9 10 11 12"
	};

	const std::vector<std::vector<std::string>> outputs = {
		{},
		{"1"},
		{"1"},
		{"1"},
		{"1", "2"},
		{"1", "2"},
		{"1", "2"},
		{"1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12"}
	};

	REQUIRE(inputs.size() == outputs.size());

	for (std::size_t i = 0; i < inputs.size(); ++i)
	{
		std::vector<std::string> result = WebservUtility::splitString(inputs[i], delim);
		REQUIRE(result == outputs[i]);
	}

	std::vector<std::string> result = WebservUtility::splitString("123", '\0');
	std::vector<std::string> exp = {"123"};
	REQUIRE(result == exp);

	result = WebservUtility::splitString("\t1\t2 3\n4 ", " \t\n");
	exp = {"1", "2", "3", "4"};
	REQUIRE(result == exp);
}
