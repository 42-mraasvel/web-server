#include "TestCases/TestCase.hpp"
#include "Utility/utility.hpp"

static TestCase defaultTestCase() {
	TestCase testcase;
	testcase.server = Address("localhost", 8080);
	testcase.tag = "Expect";
	return testcase;
}

static Request::Pointer defaultRequest() {
	Request::Pointer request(new Request);
	request->request_line = "GET / HTTP/1.1";
	request->header_fields["Host"] = "localhost";
	request->header_fields["Expect"] = "100-continue";
	return request;
}

static bool expectValidator(const std::vector<Response::Pointer>& response, const Response::Pointer expected) {
	if (response.size() != 2) {
		return false;
	}
	return response.front()->status_code == 100 && response.back()->status_code == 200;
}

TestCase testCaseExpect() {
	Request::Pointer request = defaultRequest();

	request->message_body = "1234";
	request->header_fields["Content-Length"] = util::itoa(request->message_body.size());
	request->settings.flags = Request::Settings::EXPECT;

	ResponseValidator validator(expectValidator, 2);
	TestCase testcase = defaultTestCase();
	testcase.requests.push_back(TestCase::RequestPair(request, validator));
	testcase.name = "100-continue";
	return testcase;
}
