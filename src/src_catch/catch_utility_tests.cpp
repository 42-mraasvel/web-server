#include "utility/utility.hpp"
#include "catch.hpp"
#include <tuple>
#include <iostream>

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) sizeof(x) / sizeof(x[0])
#endif

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

TEST_CASE("findLimit basic tests", "[utility]")
{
	//tuple<a, b, c, d>
	// findLimit(a, b, c) == d;
	std::tuple<std::string, std::string, std::size_t, std::size_t> inputs[] = {
		{"12345", "1", 1, 0},
		{"12345", "2", 1, std::string::npos},
		{"12345", "2", 2, 1},
		{"12345", "234", 2, std::string::npos},
		{"12345", "234", 3, std::string::npos},
		{"12345", "234", 4, 1},
		{"GET / HTTP/1.1\r\n", "\r\n", 8000, 14},
		{"GET / HTTP/1.1\r\n\r\n", "\r\n\r\n", 8000, 14}
	};

	for(std::size_t i = 0; i < ARRAY_SIZE(inputs); ++i)
	{
		auto x = inputs[i];
		REQUIRE(WebservUtility::findLimit(
			std::get<0>(x),
			std::get<1>(x),
			std::get<2>(x)) == std::get<3>(x));
	}
}

TEST_CASE("strtol basic", "[utility]")
{
	std::pair<std::string, long> inputs[] = {
		{"1", 1},
		{"0", 0},
		{"-1", -1},
		{"+1", 1},
		{"a1234235", 0},
		{"", 0},
		{"2983749817948719823479817231", 0},
		{"9223372036854775807", 9223372036854775807l},
		{"-9223372036854775808", INT64_MIN},
		{"-9223372036854775809", 0},
		{"18446744073709551615", 0},
		{"2147483647", 2147483647},
		{"-2147483648", -2147483648}
	};

	for (std::size_t i = 0; i < ARRAY_SIZE(inputs); ++i)
	{
		REQUIRE(WebservUtility::strtol(inputs[i].first) == inputs[i].second);
	}
}

TEST_CASE("convertToLowercase utility", "[utility]")
{
	std::pair<std::string, std::string> inputs[] = {
		{"1234", "1234"},
		{"abcde", "abcde"},
		{"aBcDe", "abcde"},
		{"a1A2", "a1a2"}
	};

	for (std::size_t i = 0; i < ARRAY_SIZE(inputs); ++i)
	{
		WebservUtility::convertToLowercase(inputs[i].first);
		REQUIRE(inputs[i].first == inputs[i].second);
	}
}

TEST_CASE("strtol: hex base", "[utility]")
{
	std::pair<std::string, long> inputs[] = {
		{"0x1234", 4660},
		{"-0x1234", -4660},
		{"-0x8000000000000000", INT64_MIN},
		{"0x0", 0},
		{"-0x0", 0},
		{"+0x0", 0},
		{"+0x1", 1},
		{"-0x1", -1},
		{"0x7FFFFFFFFFFFFFFF", INT64_MAX},
	};

	for (std::size_t i = 0; i < ARRAY_SIZE(inputs); ++i)
	{
		long answer;
		WebservUtility::strtol(inputs[i].first, answer, 16);
		REQUIRE(answer == inputs[i].second);
	}
}

TEST_CASE("strtoul: hex base", "[utility]")
{
	std::pair<std::string, unsigned long> inputs[] = {
		{"0x1234", 4660},
		{"0xFFFFFFFFFFFFFFFF", UINT64_MAX},
		{"0x1", 1},
		{"0x0", 0},
		{"+0x0", 0},
	};

	for (std::size_t i = 0; i < ARRAY_SIZE(inputs); ++i)
	{
		unsigned long answer;
		// answer = std::strtoul(inputs[i].first.c_str(), NULL, 16);
		REQUIRE(WebservUtility::strtoul(inputs[i].first, answer, 16) == 0);
		REQUIRE(answer == inputs[i].second);
	}
}

TEST_CASE("strtol: overflow", "[utility]")
{
	const std::string inputs_b10[] = {
		"12983719273981273981729381729837"
	};

	for (std::size_t i = 0; i < ARRAY_SIZE(inputs_b10); ++i)
	{
		long n;
		REQUIRE(WebservUtility::strtol(inputs_b10[i], n, 10) == -1);
	}
}

TEST_CASE("stringEndsWith", "[utility]")
{
	REQUIRE(WebservUtility::stringEndsWith("1234", "4"));
	REQUIRE(WebservUtility::stringEndsWith("1234", "2", 0, 2));
	REQUIRE(WebservUtility::stringEndsWith("1234", "3", 0, 3));
	REQUIRE(WebservUtility::stringEndsWith("/z.py", ".py", 0, std::string::npos));
	REQUIRE(WebservUtility::stringEndsWith("/x/y.py/a/b/c", ".py", 2, 7));
	REQUIRE(!WebservUtility::stringEndsWith("/x/y.pyz/a/b/c", ".py", 2, 8));
}
