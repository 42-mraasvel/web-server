#include "TestCases/TestCase.hpp"

static TestCase defaultTestCase() {
	TestCase testcase;
	testcase.server = Address("localhost", 8080);
	testcase.settings.flags |= Client::Settings::WAIT_FOR_CLOSE;
	testcase.settings.timeout = 120;
	testcase.execute_only_if_tag = true;
	testcase.tag = "Timeout";
	testcase.name = "Wait";
	return testcase;
}

TestCase testCaseTimeout() {
	TestCase testcase = defaultTestCase();
	return testcase;
}

TestCase testCaseTimeoutWithRequest() {
	Request::Pointer request(new Request);
	Response::Pointer response(new Response);

	request->request_line = "GET / HTTP/1.1";
	request->header_fields["host"] = "localhost";
	response->status_code = 200;

	TestCase testcase = defaultTestCase();
	testcase.requests.push_back(TestCase::RequestPair(request, ResponseValidator(response)));
	return testcase;
}
