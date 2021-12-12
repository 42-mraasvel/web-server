#include "ResponseValidator.hpp"
#include "DataStructures/Request.hpp"
#include "Utility/Output.hpp"
#include "color.hpp"

bool validateStatusCode(const Response& response, const Response& expected) {
	return response.status_code == expected.status_code;
}

bool validateHeaderFields(const Response& response, const Response& expected) {
	if (!validateStatusCode(response, expected)) {
		return false;
	}
	for (auto it = expected.header_fields.begin(); it != expected.header_fields.end(); ++it) {
		auto field = response.header_fields.get(it->first);
		if (!field.second) {
			return false;
		} else if (field.first->second != it->second) {
			return false;
		}
	}
	return true;
}

bool validateAll(const Response& response, const Response& expected) {
	if (!validateHeaderFields(response, expected)) {
		return false;
	}
	return response.message_body == expected.message_body;
}

ResponseValidator::ResponseValidator(Response::Pointer expected, ValidatorFunction validator)
: expected(expected), validator(validator) {}

bool ResponseValidator::isValidResponse(const Response& response) {
	return validator(response, *expected);
}

/*
Output functions
*/
void ResponseValidator::fail(const Request& request, const Response& response) const {
	PRINT << RED_BOLD << "Fail" RESET_COLOR ": [" << request.name << "-" << request.tag << "]" << std::endl;
	LOG_ERR << "Failed Testcase: [" << request.name << "-" << request.tag << "]" << std::endl;
	LOG_INFO << "Sent Request" << std::endl;
	request.log();
	LOG_INFO << "Received Response" << std::endl;
	response.log();
	LOG_INFO << "Expected Response" << std::endl;
	expected->log();
}

void ResponseValidator::pass(const Request& request, const Response& response) const {
	PRINT << GREEN_BOLD << "Pass" RESET_COLOR ": [" << request.name << "-" << request.tag << "]" << std::endl;
}

void ResponseValidator::print() const {
	PRINT_DEBUG << "-- Validator --" << std::endl;
	expected->print();
}
