#include "TestCases/TestCase.hpp"

static TestCase defaultTestCase() {
	TestCase testcase;
	testcase.server = Address("localhost", 8080);
	testcase.tag = "Stress";
	return testcase;
}

static Request::Pointer defaultRequest() {
	Request::Pointer request(new Request);
	request->request_line = "GET / HTTP/1.1";
	request->header_fields["host"] = "localhost";
	return request;
}

static Response::Pointer defaultResponse() {
	Response::Pointer response(new Response);
	response->status_code = StatusCode::STATUS_OK;
	return response;
}

TestCase defaultStressTest() {
	TestCase testcase = defaultTestCase();

	testcase.settings.flags |= Client::Settings::PIPELINED;
	testcase.settings.pipeline_amount = std::numeric_limits<int>::max();
	testcase.name = "Stress";
	testcase.execute_only_if_tag = true;
	ResponseValidator validator(defaultResponse());
	int n = 650;
	for (int i = 0; i < n; ++i) {
		testcase.requests.push_back(TestCase::RequestPair(defaultRequest(), validator));
	}
	return testcase;
}

TestCase testCaseStress0() {
	return defaultStressTest();
}
