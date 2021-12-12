#include "TestCase.hpp"
#include "Client/Client.hpp"

static void executeTestCase(const TestCase& testcase) {
	Client::testRequests(testcase.requests, testcase.server, testcase.settings);
}

static void nameRequests(TestCase& testcase) {
	for (TestCase::RequestPair request_pair : testcase.requests) {
		request_pair.first->name = testcase.name;
		request_pair.first->tag = testcase.tag;
	}
}

void executeTestCases(TestCaseVector testcases, const TagSet tags) {
	for (TestCase& testcase : testcases) {
		if (tags.size() > 0 && tags.count(testcase.tag) == 0) {
			continue;
		}
		nameRequests(testcase);
		executeTestCase(testcase);
	}
}
