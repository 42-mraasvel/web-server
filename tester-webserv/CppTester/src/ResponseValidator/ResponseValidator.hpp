#pragma once

#include "DataStructures/Response.hpp"
#include <memory>

bool validateStatusCode(const Response& response, const Response& expected);
bool validateHeaderFields(const Response& response, const Response& expected);
bool validateAll(const Response& response, const Response& expected);

struct Request;

class ResponseValidator {
	public:
		typedef bool (*ValidatorFunction)(const Response& response, const Response& expected);

	public:
		ResponseValidator(Response::Pointer expected,
					ValidatorFunction validator = &validateHeaderFields);

		bool isValidResponse(const Response& response);

		void fail(const Request& request, const Response& response) const;
		void pass(const Request& request, const Response& response) const;
		void print() const;

	private:
		Response::Pointer expected;
		ValidatorFunction validator;
};
