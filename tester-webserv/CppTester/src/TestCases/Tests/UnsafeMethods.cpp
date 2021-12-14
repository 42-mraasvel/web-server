#include "TestCases/TestCase.hpp"

namespace _detailed_ {
static const std::string REQUEST_TARGET = "/Unsafe/filename.txt";
}

static TestCase defaultTestCase() {
	TestCase testcase;
	testcase.server = Address("localhost", 8080);
	testcase.tag = "Unsafe";
	testcase.settings.flags |= Client::Settings::PIPELINED;
	testcase.settings.pipeline_amount = 100;
	return testcase;
}

static Request::Pointer defaultRequest() {
	Request::Pointer request(new Request);
	request->request_line = "GET / HTTP/1.0";
	return request;
}

static void addToTestCase(TestCase& testcase, Request::Pointer request, Response::Pointer response) {
	testcase.requests.push_back(TestCase::RequestPair(request, ResponseValidator(response)));
}

static std::string generateRequestLine(const std::string& method) {
	return method + " " + _detailed_::REQUEST_TARGET + " HTTP/1.1";
}

static std::string postMessageBody() {
	size_t n = 100000;
	std::string body(n, 'a');
	body.append("\n");
	return body;
}

static Request::Pointer targetGet() {
	Request::Pointer get(new Request);
	get->request_line = generateRequestLine("GET");
	return get;
}

static TestCase::RequestPair invalidGet() {
	Request::Pointer get =  targetGet();
	get->name = "invalid get";
	Response::Pointer expected(new Response);
	expected->status_code = 404;
	return TestCase::RequestPair(get, ResponseValidator(expected));
}

static TestCase::RequestPair validGet() {
	Request::Pointer get =  targetGet();
	get->name = "valid get";
	Response::Pointer expected(new Response);
	expected->status_code = 200;
	expected->message_body = postMessageBody();
	return TestCase::RequestPair(get, ResponseValidator(expected, &validateAll));
}

static TestCase::RequestPair postRequest() {
	Request::Pointer post(new Request);
	post->name = "post";
	post->request_line = generateRequestLine("POST");
	post->message_body = postMessageBody();
	post->header_fields["Content-Length"] = std::to_string(post->message_body.size());

	Response::Pointer expected(new Response);
	expected->status_code = 201;
	return TestCase::RequestPair(post, ResponseValidator(expected));
}

static TestCase::RequestPair deleteRequest() {
	Request::Pointer del(new Request);
	del->name = "delete";
	del->request_line = generateRequestLine("DELETE");

	Response::Pointer expected(new Response);
	expected->status_code = 204;
	return TestCase::RequestPair(del, ResponseValidator(expected));
}

TestCase testCaseUnsafe() {
	TestCase testcase = defaultTestCase();
	testcase.name = "Get->Post->Get->Delete->Get";

	testcase.requests.push_back(invalidGet());
	testcase.requests.push_back(postRequest());
	testcase.requests.push_back(validGet());
	testcase.requests.push_back(deleteRequest());
	testcase.requests.push_back(invalidGet());

	for (auto it : testcase.requests) {
		it.first->header_fields["Host"] = "unsafe_method";
	}
	return testcase;
}
