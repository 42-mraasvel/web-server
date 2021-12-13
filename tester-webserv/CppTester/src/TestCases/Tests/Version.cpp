#include "TestCases/TestCase.hpp"
#include "Utility/status_codes.hpp"

TestCase testCaseVersionNotSupported() {
	TestCase testcase;
	testcase.server = Address("localhost", 8080);
	testcase.tag = "Version";

	std::vector<int> maj_versions = {2, 100};

	testcase.name = "Not Supported: Major";
	Response::Pointer expected(new Response);
	expected->status_code = StatusCode::HTTP_VERSION_NOT_SUPPORTED;
	for (int version : maj_versions) {
		Request::Pointer request(new Request);
		request->request_line = "GET / HTTP/" + std::to_string(version) + ".0";
		testcase.requests.push_back(TestCase::RequestPair(request, ResponseValidator(expected)));
	}
	return testcase;
}
