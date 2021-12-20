#include "TestCases/TestCase.hpp"

static Response::Pointer defaultExpected() {
	Response::Pointer expected(new Response);
	expected->status_code = StatusCode::REQUEST_HEADER_FIELDS_TOO_LARGE;
	return expected;
}

static TestCase::RequestPair defaultRequestPair(Request::Pointer request) {
	return TestCase::RequestPair(request, ResponseValidator(defaultExpected()));
}

static Request::Pointer defaultRequest() {
	Request::Pointer request(new Request);
	request->request_line = "GET / HTTP/1.1";
	request->header_fields["host"] = "localhost";
	request->header_fields["bla"] = "bla";
	return request;
}

static TestCase defaultTestCase() {
	TestCase testcase;
	testcase.tag = "RequestHeaderField";
	testcase.name = "TOO LARGE";
	return testcase;
}

TestCase testCaseLargeHeaderField() {
	Request::Pointer single_field = defaultRequest();
	single_field->header_fields["too_long"] = std::string(1024 * 1024, 'a');

	TestCase testcase = defaultTestCase();
	testcase.requests.push_back(defaultRequestPair(single_field));
	return testcase;
}

TestCase testCaseTooManyHeaderField() {
	Request::Pointer many = defaultRequest();
	for (size_t i = 0; i < 2501; ++i) {
		many->header_fields[std::to_string(i)] = "a";
	}

	TestCase testcase = defaultTestCase();
	testcase.requests.push_back(defaultRequestPair(many));
	return testcase;
}
