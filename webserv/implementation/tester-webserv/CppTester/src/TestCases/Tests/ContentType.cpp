#include "TestCases/TestCase.hpp"

static TestCase defaultTestCase(TestCase& testcase) {
	testcase.tag = "ContentType";
	for (auto it : testcase.requests) {
		it.first->header_fields["host"] = "content_type";
	}
	return testcase;
}

static std::string getRequestLine(const std::string& filename) {
	return "GET /" + filename + " HTTP/1.1";
}

static bool contentTypeValidator(const std::vector<Response::Pointer>& response, const Response::Pointer expected) {
	if (response.size() != 1
	|| !response.front()->header_fields.contains("content-type")
	|| !validateStatusCode(response, expected)) {
		return false;
	}

	std::string value = response.front()->header_fields.get("content-type").first->second;
	std::size_t find = value.find(";");
	if (find != std::string::npos) {
		value.erase(value.begin() + find, value.end());
	}
	return expected->header_fields["content-type"] == value;
}

static TestCase::RequestPair defaultRequest(const std::string& filename, const std::string content_type) {
	Request::Pointer request(new Request);
	request->request_line = getRequestLine(filename);

	Response::Pointer expected(new Response);
	expected->status_code = StatusCode::STATUS_OK;
	expected->header_fields["Content-Type"] = content_type;

	return TestCase::RequestPair(request, ResponseValidator(expected, contentTypeValidator));
}

TestCase testCaseContentType() {
	TestCase testcase;

	testcase.requests.push_back(defaultRequest("index.html", "text/html"));
	testcase.requests.push_back(defaultRequest("index.txt", "text/plain"));
	testcase.requests.push_back(defaultRequest("program.py", "text/x-script.python"));
	testcase.requests.push_back(defaultRequest("noext", "application/octet-stream"));

	testcase.name = "Extensions";
	return defaultTestCase(testcase);
}
