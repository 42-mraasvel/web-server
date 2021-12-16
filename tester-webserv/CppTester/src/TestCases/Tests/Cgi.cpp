#include "TestCases/TestCase.hpp"

static TestCase defaultTestCase(TestCase& testcase) {
	testcase.server = Address("localhost", 8080);
	testcase.tag = "CGI";
	for (auto it : testcase.requests) {
		it.first->header_fields["host"] = "CgiBlock";
	}
	return testcase;
}

static Request::Pointer defaultPostRequest() {
	Request::Pointer request(new Request);
	request->request_line = "POST / HTTP/1.1";
	return request;
}

static std::string getMessageBody(std::size_t n) {
	return std::string(n, 'a');
}

TestCase testCaseCgiMessageBody() {
	TestCase testcase;

	std::size_t n = 500;

	Request::Pointer request = defaultPostRequest();
	request->message_body = "MessageBody\r\n" + getMessageBody(n);
	request->header_fields["Content-Length"] = std::to_string(request->message_body.size());

	Response::Pointer expected(new Response);
	expected->status_code = 200;
	expected->message_body = getMessageBody(n);

	testcase.requests.push_back(TestCase::RequestPair(request, ResponseValidator(expected, &validateAll)));
	testcase.name = "Message Body";
	return defaultTestCase(testcase);
}

TestCase testCaseCgiStatusCode() {
	TestCase testcase;

	Response::Pointer expected(new Response);
	expected->status_code = 666;

	Request::Pointer request = defaultPostRequest();
	request->message_body = "StatusCode\r\n" + std::to_string(expected->status_code);
	request->header_fields["Content-Length"] = std::to_string(request->message_body.size());

	testcase.requests.push_back(TestCase::RequestPair(request, ResponseValidator(expected)));
	testcase.name = "Status Code";
	return defaultTestCase(testcase);
}

TestCase testCaseCgiBadSyntax() {
	TestCase testcase;

	Response::Pointer expected(new Response);
	expected->status_code = StatusCode::BAD_GATEWAY;

	Request::Pointer request = defaultPostRequest();
	request->message_body = "BadSyntax\r\n" + std::to_string(expected->status_code);
	request->header_fields["Content-Length"] = std::to_string(request->message_body.size());

	testcase.requests.push_back(TestCase::RequestPair(request, ResponseValidator(expected)));
	testcase.name = "Bad Syntax";
	return defaultTestCase(testcase);
}

TestCase testCaseCgiCrash() {
	TestCase testcase;

	Response::Pointer expected(new Response);
	expected->status_code = StatusCode::BAD_GATEWAY;

	Request::Pointer request = defaultPostRequest();
	request->message_body = "Crash\r\n" + std::to_string(expected->status_code);
	request->header_fields["Content-Length"] = std::to_string(request->message_body.size());

	testcase.requests.push_back(TestCase::RequestPair(request, ResponseValidator(expected)));
	testcase.name = "Crash";
	return defaultTestCase(testcase);
}

TestCase testCaseCgiNonExistantFile() {
	TestCase testcase;

	Response::Pointer expected(new Response);
	expected->status_code = StatusCode::BAD_GATEWAY;

	Request::Pointer request = defaultPostRequest();
	request->request_line = "POST /test.nonexistant HTTP/1.1";
	testcase.requests.push_back(TestCase::RequestPair(request, ResponseValidator(expected)));
	testcase.name = "Non Existant";
	return defaultTestCase(testcase);
}

TestCase testCaseCgiNoPermission() {
	TestCase testcase;

	Response::Pointer expected(new Response);
	expected->status_code = StatusCode::BAD_GATEWAY;

	Request::Pointer request = defaultPostRequest();
	request->request_line = "POST /test.noperm HTTP/1.1";
	testcase.requests.push_back(TestCase::RequestPair(request, ResponseValidator(expected)));
	testcase.name = "No Permission";
	return defaultTestCase(testcase);
}

TestCase testCaseCgiTimeout() {
	TestCase testcase;

	Response::Pointer expected(new Response);
	expected->status_code = StatusCode::GATEWAY_TIMEOUT;

	std::size_t seconds_to_wait = 100; // Timeout option might differ per webserver implementation
	Request::Pointer request = defaultPostRequest();
	request->message_body = "Timeout\r\n" + std::to_string(seconds_to_wait);
	request->header_fields["Content-Length"] = std::to_string(request->message_body.size());

	testcase.requests.push_back(TestCase::RequestPair(request, ResponseValidator(expected)));
	testcase.name = "Timeout";
	testcase.settings.timeout = 120;
	testcase.execute_only_if_tag = true;
	return defaultTestCase(testcase);
}
