#include "TestCases/TestCase.hpp"
#include <string>

static TestCase defaultTestCase() {
	TestCase testcase;
	testcase.server = Address("localhost", 8080);
	testcase.tag = "Redirection";
	return testcase;
}

static Request::Pointer defaultRequest(int n) {
	Request::Pointer request(new Request);
	request->request_line = "GET /" + std::to_string(n) + " HTTP/1.1";
	request->header_fields["Host"] = "locationblock_redirection";
	return request;
}

static Response::Pointer defaultResponse(int n) {
	Response::Pointer expected(new Response);
	expected->status_code = n;
	expected->header_fields["Location"] = "http://localhost:8080/" + std::to_string(n);
	// expected->header_fields["Retry-After"] = "30";
	return expected;
}

static TestCase::RequestPair defaultPair(int n) {
	Request::Pointer request = defaultRequest(n);
	ResponseValidator validator(defaultResponse(n));
	return TestCase::RequestPair(request, validator);
}

TestCase testCaseRedirection301() {
	TestCase testcase = defaultTestCase();
	testcase.name = "301";
	testcase.requests.push_back(TestCase::RequestPair(defaultPair(301)));
	return testcase;
};

TestCase testCaseRedirection308() {
	TestCase testcase = defaultTestCase();
	testcase.name = "308";
	testcase.requests.push_back(TestCase::RequestPair(defaultPair(308)));
	return testcase;
};

TestCase testCaseRedirection302() {
	TestCase testcase = defaultTestCase();
	testcase.name = "302";
	testcase.requests.push_back(TestCase::RequestPair(defaultPair(302)));
	return testcase;
};

TestCase testCaseRedirection303() {
	TestCase testcase = defaultTestCase();
	testcase.name = "303";
	testcase.requests.push_back(TestCase::RequestPair(defaultPair(303)));
	return testcase;
};

TestCase testCaseRedirection307() {
	TestCase testcase = defaultTestCase();
	testcase.name = "307";
	testcase.requests.push_back(TestCase::RequestPair(defaultPair(307)));
	return testcase;
};
