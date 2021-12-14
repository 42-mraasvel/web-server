#include "TestCases/TestCase.hpp"

static TestCase defaultTestCase() {
	TestCase testcase;
	testcase.server = Address("localhost", 8080);
	testcase.tag = "Connection";
	return testcase;
}

static Request::Pointer defaultRequest() {
	Request::Pointer request(new Request);
	request->request_line = "GET / HTTP/1.0";
	return request;
}

TestCase testCaseConnectionDefault0() {
	Request::Pointer request = defaultRequest();

	Response::Pointer expected(new Response);
	expected->status_code = StatusCode::STATUS_OK;
	expected->header_fields["connection"] = "close";

	TestCase testcase = defaultTestCase();
	testcase.name = "1.0: close";
	testcase.settings.flags |= Client::Settings::WAIT_FOR_CLOSE;
	testcase.settings.wait_close = 20;
	testcase.execute_only_if_tag = true;
	testcase.requests.push_front(TestCase::RequestPair(request, ResponseValidator(expected)));
	return testcase;
}

TestCase testCaseConnectionKeepAlive() {
	Request::Pointer request = defaultRequest();
	request->header_fields["connection"] = "keep-alive";

	Response::Pointer expected(new Response);
	expected->status_code = StatusCode::STATUS_OK;
	expected->header_fields["connection"] = "keep-alive";

	TestCase testcase = defaultTestCase();
	testcase.requests.push_front(TestCase::RequestPair(request, ResponseValidator(expected)));
	testcase.name = "1.0: Keep-Alive";
	return testcase;
}

TestCase testCaseConnectionClose() {
	Request::Pointer request = defaultRequest();
	request->request_line = "GET / HTTP/1.1";
	request->header_fields["host"] = "random";
	request->header_fields["connection"] = "close";

	Response::Pointer expected(new Response);
	expected->status_code = StatusCode::STATUS_OK;
	expected->header_fields["connection"] = "close";

	TestCase testcase = defaultTestCase();
	testcase.name = "1.1: Close";
	testcase.settings.flags |= Client::Settings::WAIT_FOR_CLOSE;
	testcase.settings.wait_close = 20;
	testcase.execute_only_if_tag = true;
	testcase.requests.push_front(TestCase::RequestPair(request, ResponseValidator(expected)));
	return testcase;
}

TestCase testCaseConnectionDefault1() {
	Request::Pointer request = defaultRequest();
	request->request_line = "GET / HTTP/1.1";
	request->header_fields["host"] = "random";

	Response::Pointer expected(new Response);
	expected->status_code = StatusCode::STATUS_OK;
	expected->header_fields["connection"] = "keep-alive";

	TestCase testcase = defaultTestCase();
	testcase.name = "1.1: keep-alive";
	testcase.requests.push_front(TestCase::RequestPair(request, ResponseValidator(expected)));
	return testcase;
}
