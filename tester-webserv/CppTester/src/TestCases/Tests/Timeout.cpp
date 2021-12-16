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

static bool timeoutValidation(const std::vector<Response::Pointer>& response, const Response::Pointer expected) {
	if (response.size() != 2) {
		return false;
	}
	return response.front()->status_code == StatusCode::STATUS_OK && response.back()->status_code == StatusCode::REQUEST_TIMEOUT; 
}

TestCase testCaseTimeoutWithRequest() {
	Request::Pointer request(new Request);
	Response::Pointer response(new Response);

	request->request_line = "GET / HTTP/1.1";
	request->header_fields["host"] = "localhost";

	TestCase testcase = defaultTestCase();
	testcase.settings.timeout = 75;
	testcase.requests.push_back(TestCase::RequestPair(request, ResponseValidator(nullptr, &timeoutValidation, 2)));
	return testcase;
}
