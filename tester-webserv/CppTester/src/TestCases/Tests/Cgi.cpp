#include "TestCases/TestCase.hpp"

static TestCase defaultTestCase() {
	TestCase testcase;
	testcase.server = Address("localhost", 8080);
	testcase.tag = "CGI";
	return testcase;
}

static Request::Pointer defaultRequest() {
	Request::Pointer request(new Request);
	request->request_line = "GET /cgi-bin/cgi.py HTTP/1.1";
	request->header_fields["host"] = "localhost";
	return request;
}

TestCase testCaseCgi() {
	TestCase testcase = defaultTestCase();
	Response::Pointer expected(new Response);
	expected->status_code = 200;
	testcase.requests.push_back(TestCase::RequestPair(defaultRequest(), ResponseValidator(expected)));
	return testcase;
}
