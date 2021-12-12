#include "TestCases/TestCase.hpp"

TestCase testCaseConnection() {
	TestCase testcase;

	testcase.server = Address("localhost", 8080);
	Request::Pointer request(new Request);
	request->tag = "Connection";
	request->name = "Connection";

	request->request_line = "GET / HTTP/1.0";

	Response::Pointer expected(new Response);
	expected->header_fields["connection"] = "close";
	expected->status_code = StatusCode::STATUS_OK;

	ResponseValidator validator(expected);

	testcase.requests.push_front(TestCase::RequestPair(request, validator));
	return testcase;
}
