#include "TestCases/TestCase.hpp"

static TestCase defaultTestCase(TestCase& testcase) {
	testcase.server = Address("localhost", 8080);
	testcase.tag = "CGI";
	for (auto it : testcase.requests) {
		it.first->header_fields["host"] = "CgiBlock";
	}
	return testcase;
}

static Request::Pointer defaultGetRequest() {
	Request::Pointer request(new Request);
	request->request_line = "GET / HTTP/1.1";
	return request;
}

static std::string getMessageBody(std::size_t n) {
	return std::string(n, 'a');
}

TestCase testCaseMessageBody() {
	TestCase testcase;

	std::size_t n = 500;

	Request::Pointer request = defaultGetRequest();
	request->message_body = "MessageBody\r\n" + getMessageBody(n);
	request->header_fields["Content-Length"] = std::to_string(request->message_body.size());

	Response::Pointer expected(new Response);
	expected->status_code = 200;
	expected->message_body = getMessageBody(n);

	testcase.requests.push_back(TestCase::RequestPair(request, ResponseValidator(expected, &validateAll)));
	testcase.name = "Message Body";
	return defaultTestCase(testcase);
}
