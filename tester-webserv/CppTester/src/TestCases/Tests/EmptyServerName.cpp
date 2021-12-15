#include "TestCases/TestCase.hpp"
#include "Utility/utility.hpp"
#include <fcntl.h>
#include <unistd.h>

static Request::Pointer defaultRequest() {
	Request::Pointer request(new Request);
	request->request_line = "GET /ServerBlock/empty_server/index.html HTTP/1.0";
	request->header_fields["connection"] = "keep-alive";
	return request;
}

TestCase testCaseEmptyServerName8082() {
	TestCase testcase;
	testcase.tag = "Resolution";
	testcase.name = "EmptyServerName8082";

	testcase.server = Address("localhost", 8082);
	Request::Pointer request = defaultRequest();

	Response::Pointer expected(new Response);
	std::string path = "../ServerRoot/ServerBlock/empty_server/index.html";
	int fd = open(path.c_str(),  O_RDONLY);
	testcase.requests.push_back(TestCase::RequestPair(request, ResponseValidator(expected)));
	char buffer[1000];
	bzero(buffer, 1000);
	read(fd, buffer, 1000);
	expected->status_code = StatusCode::STATUS_OK;
	expected->message_body = std::string(buffer);
	expected->header_fields["Content-Length"] = std::to_string(expected->message_body.size());

	return testcase;
}

TestCase testCaseEmptyServerName8083() {
	TestCase testcase;
	testcase.tag = "Resolution";
	testcase.name = "EmptyServerName8083";

	testcase.server = Address("localhost", 8083);
	Request::Pointer request = defaultRequest();

	Response::Pointer expected(new Response);
	std::string path = "../ServerRoot/ServerBlock/empty_server/index.html";
	int fd = open(path.c_str(),  O_RDONLY);
	testcase.requests.push_back(TestCase::RequestPair(request, ResponseValidator(expected)));
	char buffer[1000];
	bzero(buffer, 1000);
	read(fd, buffer, 1000);
	expected->status_code = StatusCode::STATUS_OK;
	expected->message_body = std::string(buffer);
	expected->header_fields["Content-Length"] = std::to_string(expected->message_body.size());

	return testcase;
}
