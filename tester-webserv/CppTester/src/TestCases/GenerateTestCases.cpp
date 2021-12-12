#include "TestCase.hpp"

TestCase testCaseConnection();

TestCaseVector generateTestCases() {
	TestCaseVector testcases;
	testcases.push_back(testCaseConnection());
	return testcases;
}
