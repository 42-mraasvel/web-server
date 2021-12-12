#include "TestCase.hpp"

TestCase testCaseConnectionDefault0();
TestCase testCaseConnectionKeepAlive();
TestCase testCaseConnectionClose();
TestCase testCaseConnectionDefault1();

TestCaseVector generateTestCases() {
	TestCaseVector testcases;
	testcases.push_back(testCaseConnectionDefault0());
	testcases.push_back(testCaseConnectionKeepAlive());
	testcases.push_back(testCaseConnectionClose());
	testcases.push_back(testCaseConnectionDefault1());
	return testcases;
}
