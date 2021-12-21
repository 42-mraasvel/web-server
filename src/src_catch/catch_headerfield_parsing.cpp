#include "catch.hpp"
#include "parser/HeaderFieldParser.hpp"
#include "settings.hpp"
#include "utility/status_codes.hpp"
#include "parser/ParserUtils.hpp"

#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))

namespace TestingDetail
{

static const std::size_t MAX_HEADER_FIELD_SIZE = 4096;

static bool validFieldFunction(const std::string& key, const std::string& value, const HeaderField& fields)
{
	(void)key;
	(void)value;
	(void)fields;
	return true;
}
static bool invalidFieldFunction(const std::string& key, const std::string& value, const HeaderField& fields)
{
	(void)key;
	(void)value;
	(void)fields;
	return false;
}

static std::string randomCase(const std::string& x) {
	std::string y;

	for (auto c : x) {
		if (rand() % 2) {
			y.push_back(toupper(c));
		} else {
			y.push_back(tolower(c));
		}
	}
	return y;
}


}

TEST_CASE("Basic HeaderFieldParser", "[header-field-parser]") {
	HeaderFieldParser parser(
			TestingDetail::validFieldFunction,
			TestingDetail::MAX_HEADER_FIELD_SIZE);
		
	static const std::string input =
		"FieldName: FieldValue" CRLF
		"F2: FV2" CRLF
		"F3: FV3" CRLF
		"F: " CRLF
		"x:x" CRLF
		"F:    x   " CRLF
		"F:    x 1 2 3  " CRLF
		CRLF;

	std::size_t index = 0;
	REQUIRE(parser.parse(input, index) != ERR);
	REQUIRE(parser.isComplete());
	parser.reset();

	for (std::size_t i = 0; i < input.size(); ++i) {
		std::size_t index = 0;
		REQUIRE(parser.parse(input.substr(i, 1), index) != ERR);
	}

	REQUIRE(parser.isComplete());
}

TEST_CASE("InvalidFieldFunction HeaderField", "[header-field-parser]") {
	HeaderFieldParser parser(
		TestingDetail::invalidFieldFunction,
		TestingDetail::MAX_HEADER_FIELD_SIZE
	);

	std::string input =
		"HeaderStart: HeaderValue" CRLF
		CRLF;

	std::size_t index = 0;
	REQUIRE(parser.parse(input, index) == ERR);
	REQUIRE(parser.isError());
	REQUIRE(parser.getStatusCode() == StatusCode::BAD_REQUEST);
	parser.reset();

	for (std::size_t i = 0; i < input.size(); ++i) {
		std::size_t index = 0;
		if (parser.parse(input.substr(i, 1), index) == ERR)
		{
			break;
		}
	}

	REQUIRE(parser.isError());
	REQUIRE(parser.getStatusCode() == StatusCode::BAD_REQUEST);
}

TEST_CASE("Small MAX_SIZE HeaderField", "[header-field-parser]") {
	HeaderFieldParser parser(
		TestingDetail::validFieldFunction,
		10
	);

	std::string input =
		"a:b" CRLF
		"asdasdasdasd: asdasdasd" CRLF // Field is longer than 10 chars
		CRLF;

	std::size_t index = 0;
	REQUIRE(parser.parse(input, index) == ERR);
	REQUIRE(parser.isError());
	REQUIRE(parser.getStatusCode() == StatusCode::REQUEST_HEADER_FIELDS_TOO_LARGE);
	parser.reset();
	for (std::size_t i = 0; i < input.size(); ++i) {
		index = 0;
		if (parser.parse(input.substr(i, 1), index) == ERR) {
			break;
		}
	}
	REQUIRE(parser.isError());
	REQUIRE(parser.getStatusCode() == StatusCode::REQUEST_HEADER_FIELDS_TOO_LARGE);
}

TEST_CASE("Name/Value checks", "[header-field-parser]") {
	HeaderFieldParser parser(
		TestingDetail::validFieldFunction,
		TestingDetail::MAX_HEADER_FIELD_SIZE
	);

	HeaderField header;

	std::string fieldnames[] = {"field1", "field2", "field3"};

	std::string input;
	for (std::size_t i = 0; i < ARRAY_SIZE(fieldnames); ++i) {
		input += fieldnames[i] + ": " + fieldnames[i] + CRLF;
	}
	input += CRLF;

	srand(time(0));

	std::size_t index = 0;
	REQUIRE(parser.parse(input, index) != ERR);
	REQUIRE(parser.isComplete());
	header.swap(parser.getHeaderField());
	for (std::size_t i = 0; i < ARRAY_SIZE(fieldnames); ++i) {
		std::string key = TestingDetail::randomCase(fieldnames[i]);
		REQUIRE(fieldnames[i] == header[key]);
	}
}
