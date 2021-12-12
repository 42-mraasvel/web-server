#include "ResponseValidator.hpp"
#include "DataStructures/Request.hpp"
#include "Utility/Output.hpp"
#include "color.hpp"
#include <cassert>

bool validateStatusCode(const ResponseValidator::ResponseVector& response, const Response::Pointer expected) {
	assert(response.size() == 1);
	assert(expected != NULL);
	return response.front()->status_code == expected->status_code;
}

bool validateHeaderFields(const ResponseValidator::ResponseVector& response, const Response::Pointer expected) {
	assert(response.size() == 1);
	assert(expected != NULL);
	if (!validateStatusCode(response, expected)) {
		return false;
	}
	for (auto it = expected->header_fields.begin(); it != expected->header_fields.end(); ++it) {
		if (expected == nullptr) {
			return true;
		}
		auto field = response.front()->header_fields.get(it->first);
		if (!field.second) {
			return false;
		} else if (field.first->second != it->second) {
			return false;
		}
	}
	return true;
}

bool validateAll(const ResponseValidator::ResponseVector& response, const Response::Pointer expected) {
	assert(response.size() == 1);
	assert(expected != NULL);
	if (!validateHeaderFields(response, expected)) {
		return false;
	}
	return response.front()->message_body == expected->message_body;
}

ResponseValidator::ResponseValidator(Response::Pointer expected,
									ValidatorFunction validator,
									std::size_t expected_responses)
: expected(expected), validator(validator), expected_responses(expected_responses) {}

ResponseValidator::ResponseValidator(ValidatorFunction validator,
									std::size_t expected_responses)
: ResponseValidator(nullptr, validator, expected_responses) {}


bool ResponseValidator::isValidResponse(const ResponseVector& response) {
	return validator(response, expected);
}

std::size_t ResponseValidator::getExpectedResponses() const {
	return expected_responses;
}

/*
Output functions
*/
void ResponseValidator::fail(const Request& request, const ResponseVector& response) const {
	PRINT << RED_BOLD << "Fail" RESET_COLOR ": [" << request.name << "-" << request.tag << "]" << std::endl;
	LOG_ERR << "Failed Testcase: [" << request.name << "-" << request.tag << "]" << std::endl;
	LOG_INFO << "Sent Request" << std::endl;
	request.log();
	LOG_INFO << "Received: " << response.size() << " responses" << std::endl;
	for (auto r : response) {
		LOG_INFO << "Received Response" << std::endl;
		r->log();
	}
	LOG_INFO << "Expected Response" << std::endl;
	expected->log();
}

void ResponseValidator::pass(const Request& request, const ResponseVector& response) const {
	PRINT << GREEN_BOLD << "Pass" RESET_COLOR ": [" << request.name << "-" << request.tag << "]" << std::endl;
}

void ResponseValidator::print() const {
	PRINT_DEBUG << "-- Validator --" << std::endl;
	expected->print();
}
