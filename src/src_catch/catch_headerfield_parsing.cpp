#include "catch.hpp"
#include "parser/HeaderFieldParser.hpp"
#include "settings.hpp"

namespace TestingDetail
{

static const std::size_t MAX_HEADER_FIELD_SIZE = 10;

bool validFieldFunction(const std::string& key, const std::string& value, const HeaderField& fields)
{
	return true;
}

}

TEST_CASE("Basic HeaderField", "[header-field]") {
	HeaderFieldParser parser(
			TestingDetail::validFieldFunction,
			TestingDetail::MAX_HEADER_FIELD_SIZE);
		
	static const std::string input =
		"FieldName: FieldValue" CRLF
		"F2: FV2" CRLF
		"F3: FV3" CRLF
		CRLF;

	std::size_t index = 0;
	REQUIRE(parser.parse(input, index) != ERR);

	// for (std::size_t i = 0; i < input.size(); ++i) {
	// 	std::size_t index = 0;
	// 	REQUIRE(parser.parse(input.substr(i, 1), index) != ERR);
	// }
}
