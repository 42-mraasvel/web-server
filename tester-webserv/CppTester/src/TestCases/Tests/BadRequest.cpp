#include "TestCases/TestCase.hpp"
#include "Utility/utility.hpp"

static TestCase defaultTestCase() {
	TestCase testcase;
	testcase.server = Address("localhost", 8080);
	testcase.tag = "BadRequest";
	return testcase;
}

static Request::Pointer defaultRequest() {
	Request::Pointer request(new Request);
	request->request_line = "GET / HTTP/1.1";
	request->header_fields["Host"] = "localhost";
	return request;
}

static Response::Pointer defaultResponse(int n) {
	Response::Pointer expected(new Response);
	expected->status_code = n;
	expected->message_body = std::to_string(n) + " " + StatusCode::getStatusMessage(n) + "\n";
	expected->header_fields["Content-length"] = std::to_string(expected->message_body.size());
	return expected;
}

TestCase testCaseBadRequestMethod() {
	TestCase testcase = defaultTestCase();
	testcase.name = "BadMethod";

	std::vector<std::string> methods_501 = {"GE.T", "3GET", "G1ET.", "GE5T", "GET0",
	"GET!", "!GET", "GE!T", "GE#T", "G$ET", "G^ET"};
	for (std::string i : methods_501 )
	{
		Request::Pointer request = defaultRequest();
		Response::Pointer expected = defaultResponse(StatusCode::NOT_IMPLEMENTED);
		request->request_line = i + " / HTTP/1.1";
		testcase.requests.push_back(TestCase::RequestPair(request, ResponseValidator(expected)));
	}

	std::vector<std::string> methods_400 = {"?GET", "G/ET", "GE;T", "GET?"};
	for (std::string i : methods_400 )
	{
		Request::Pointer request = defaultRequest();
		Response::Pointer expected = defaultResponse(StatusCode::BAD_REQUEST);
		request->request_line = i + " / HTTP/1.1";
		testcase.requests.push_back(TestCase::RequestPair(request, ResponseValidator(expected)));
	}
	return testcase;
}

TestCase testCaseBadRequestTarget() {
	TestCase testcase = defaultTestCase();
	testcase.name = "BadTarget";

	std::vector<std::string> target = {"./aa", "../aa", "???", "aaa/aa", "/.....", "/%#@$#^"};
	for (std::string i : target )
	{
		Request::Pointer request = defaultRequest();
		Response::Pointer expected = defaultResponse(StatusCode::BAD_REQUEST);
		request->request_line = "GET " + i + " HTTP/1.1";
		testcase.requests.push_back(TestCase::RequestPair(request, ResponseValidator(expected)));
	}

	return testcase;
}

TestCase testCaseBadRequestVersion() {
	TestCase testcase = defaultTestCase();
	testcase.name = "BadVersion";

	std::vector<std::string> version = {"random", "http", "/HTTP", "HTTP", "HTTP/", "HTTP///", "HTTP/aaa", "HTTP/1", "HTTP/111", "HTTP/1??", "HTTP/1...", "HTTP/1.a", "HTTP/1.?", "HTTP/1.1;"};
	for (std::string i : version )
	{
		Request::Pointer request = defaultRequest();
		Response::Pointer expected = defaultResponse(StatusCode::BAD_REQUEST);
		request->request_line = "GET / " + i;
		testcase.requests.push_back(TestCase::RequestPair(request, ResponseValidator(expected)));
	}

	return testcase;
}

TestCase testCaseBadRequestVersion() {
	TestCase testcase = defaultTestCase();
	testcase.name = "Trying";

	std::vector<std::string> version = {"random", "http", "/HTTP", "HTTP", "HTTP/", "HTTP///", "HTTP/aaa", "HTTP/1", "HTTP/111", "HTTP/1??", "HTTP/1...", "HTTP/1.a", "HTTP/1.?", "HTTP/1.1;"};
	for (std::string i : version )
	{
		Request::Pointer request = defaultRequest();
		Response::Pointer expected = defaultResponse(StatusCode::BAD_REQUEST);
		request->request_line = "GET / " + i;
		testcase.requests.push_back(TestCase::RequestPair(request, ResponseValidator(expected)));
	}

	return testcase;
}
