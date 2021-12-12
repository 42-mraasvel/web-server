#include "TestCase.hpp"
#include "Client/Client.hpp"

static void executeTestCase(const TestCase& testcase) {
	Client::testRequests(testcase.requests, testcase.server, testcase.settings);
}

void executeTestCases(TestCaseVector testcases) {
	for (const TestCase& testcase : testcases) {
		executeTestCase(testcase);
	}
}
