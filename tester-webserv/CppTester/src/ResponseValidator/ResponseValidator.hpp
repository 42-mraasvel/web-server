#pragma once

#include "DataStructures/Response.hpp"
#include <vector>
#include <memory>

bool validateStatusCode(const std::vector<Response::Pointer>& response, const Response::Pointer expected);
bool validateHeaderFields(const std::vector<Response::Pointer>& response, const Response::Pointer expected);
bool validateAll(const std::vector<Response::Pointer>& response, const Response::Pointer expected);

struct Request;

class ResponseValidator {
	public:
		typedef std::vector<Response::Pointer> ResponseVector;
		typedef bool (*ValidatorFunction)(const ResponseVector& response, const Response::Pointer expected);

	public:
		ResponseValidator(Response::Pointer expected,
					ValidatorFunction validator = &validateHeaderFields,
					std::size_t expected_responses = 1);
		ResponseValidator(ValidatorFunction validator = &validateHeaderFields,
						std::size_t expected_responses = 1);

		bool isValidResponse(const ResponseVector& response);
		std::size_t getExpectedResponses() const;

		void fail(const Request& request, const ResponseVector& response) const;
		void pass(const Request& request, const ResponseVector& response) const;
		void print() const;

	private:
		Response::Pointer expected;
		ValidatorFunction validator;
		std::size_t expected_responses;
};
