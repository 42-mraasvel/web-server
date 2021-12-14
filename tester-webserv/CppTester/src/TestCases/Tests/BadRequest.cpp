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
	"GET!", "!GET", "GE!T", "GE#T", "G$ET", "G^ET", "get", "post", "delete", "GeT"};
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

	std::vector<std::string> target = {"./aa", "../aa", "???", "aaa/aa", "/.....", "/%#@$#^", "/index.html "};
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

	std::vector<std::string> version = {"random", "http", "/HTTP", "HTTP", "HTTP/", "HTTP///", "HTTP/aaa", "HTTP/1", "HTTP/111", "HTTP/1??", "HTTP/1...", "HTTP/1.a", "HTTP/1.?", "HTTP/1.1;", "HTTP /1.1;", "http/1.1"};
	for (std::string i : version )
	{
		Request::Pointer request = defaultRequest();
		Response::Pointer expected = defaultResponse(StatusCode::BAD_REQUEST);
		request->request_line = "GET / " + i;
		testcase.requests.push_back(TestCase::RequestPair(request, ResponseValidator(expected)));
	}

	return testcase;
}

TestCase testCaseBadRequestFirstLineDelimeter() {
	TestCase testcase = defaultTestCase();
	testcase.name = "FirstLineBadDelimeter";

	std::vector<std::string> space = {"  ", "	", " 	", "		", "\n", "...", "^^", "/"};
	for (std::string i : space )
	{
		Request::Pointer request = defaultRequest();
		Response::Pointer expected = defaultResponse(StatusCode::BAD_REQUEST);
		request->request_line = "GET" + i + "/" + i + "HTTP/1.1";
		testcase.requests.push_back(TestCase::RequestPair(request, ResponseValidator(expected)));
	}

	std::vector<std::string> end = {" ", "	", "\n", ";", ";\n", "."};
	for (std::string i : end )
	{
		Request::Pointer request = defaultRequest();
		Response::Pointer expected = defaultResponse(StatusCode::BAD_REQUEST);
		request->request_line = "GET / HTTP/1.1" + i;
		testcase.requests.push_back(TestCase::RequestPair(request, ResponseValidator(expected)));
	}
	return testcase;
}

TestCase testCaseBadRequestURITooLong() {
	TestCase testcase = defaultTestCase();
	testcase.name = "FirstLineURITooLong";

	Request::Pointer request = defaultRequest();
	Response::Pointer expected = defaultResponse(StatusCode::URI_TOO_LONG);
	request->request_line.append(std::string(10000, 'x'));
	testcase.requests.push_back(TestCase::RequestPair(request, ResponseValidator(expected)));

	return testcase;
}

TestCase testCaseBadRequestHost() {
	TestCase testcase = defaultTestCase();
	testcase.name = "BadHost";

	std::vector<std::string> host = {"localhost:8081", "localhost:", "localhost;;;", "localhost::", "", "localhost:8080:", "localhost:8080:0", "localhost:8080;", "localhost :8080", "localhost: 8080"};
	for (std::string i : host )
	{
		Request::Pointer request = defaultRequest();
		Response::Pointer expected = defaultResponse(StatusCode::BAD_REQUEST);
		request->header_fields["Host"] = i;
		testcase.requests.push_back(TestCase::RequestPair(request, ResponseValidator(expected)));
	}

	// EmptyHost
	Request::Pointer request = defaultRequest();
	Response::Pointer expected = defaultResponse(StatusCode::BAD_REQUEST);
	request->header_fields.erase("Host");
	testcase.requests.push_back(TestCase::RequestPair(request, ResponseValidator(expected)));

	return testcase;
}

TestCase testCaseBadRequestMultiHost() {
	TestCase testcase = defaultTestCase();
	testcase.name = "Multiple Host";

	Request::Pointer request = defaultRequest();
	Response::Pointer expected = defaultResponse(StatusCode::BAD_REQUEST);
	std::size_t n = 10;
	for (std::size_t i = 0; i < n; ++i)
	{
		request->multi_fields.insert(std::make_pair("host", "localhost"));
	}
	testcase.requests.push_back(TestCase::RequestPair(request, ResponseValidator(expected)));
	return testcase;
}

TestCase testCaseBadRequestConnection() {
	TestCase testcase = defaultTestCase();
	testcase.name = "BadConnection";

	std::vector<std::string> connection = {"lkjl", "", "closee", " ", "close;", "keep-alive;"};
	for (std::string i : connection )
	{
		Request::Pointer request = defaultRequest();
		Response::Pointer expected = defaultResponse(StatusCode::BAD_REQUEST);
		request->header_fields["Connection"] = i;
		testcase.requests.push_back(TestCase::RequestPair(request, ResponseValidator(expected)));
	}

	return testcase;
}

TestCase testCaseBadRequestContentLength() {
	TestCase testcase = defaultTestCase();
	testcase.name = "BadContentLength";

	std::vector<std::string> connection = {"1234;", ";", "aaaa", "???", "1234.12312", "123,4124", "42, 23", "42, 42"};
	for (std::string i : connection )
	{
		Request::Pointer request = defaultRequest();
		Response::Pointer expected = defaultResponse(StatusCode::BAD_REQUEST);
		request->header_fields["Content-Length"] = i;
		testcase.requests.push_back(TestCase::RequestPair(request, ResponseValidator(expected)));
	}

	return testcase;
}

TestCase testCaseBadRequestHeader() {
	TestCase testcase = defaultTestCase();
	testcase.name = "BadHeader";

	std::vector<std::string> header = {"Content-type"};
	for (std::string i : header )
	{
		Request::Pointer request = defaultRequest();
		Response::Pointer expected = defaultResponse(StatusCode::BAD_REQUEST);
		request->header_fields[i] = ";";
		testcase.requests.push_back(TestCase::RequestPair(request, ResponseValidator(expected)));
	}

	Request::Pointer request = defaultRequest();
	Response::Pointer expected = defaultResponse(StatusCode::BAD_REQUEST);
	request->header_fields["Content-length "] = "";
	testcase.requests.push_back(TestCase::RequestPair(request, ResponseValidator(expected)));

	return testcase;
}