#include "TestCase.hpp"

TestCase testCaseConnectionDefault0();
TestCase testCaseConnectionKeepAlive();
TestCase testCaseConnectionClose();
TestCase testCaseConnectionDefault1();
TestCase testCaseExpect();
TestCase testCaseRedirection301();
TestCase testCaseRedirection308();
TestCase testCaseRedirection302();
TestCase testCaseRedirection303();
TestCase testCaseRedirection307();
TestCase testCaseStress();
TestCase testCaseVersionNotSupported();

TestCaseVector generateTestCases() {
	TestCaseVector testcases;
	testcases.push_back(testCaseConnectionDefault0());
	testcases.push_back(testCaseConnectionKeepAlive());
	testcases.push_back(testCaseConnectionClose());
	testcases.push_back(testCaseConnectionDefault1());
	testcases.push_back(testCaseExpect());
	testcases.push_back(testCaseRedirection301());
	testcases.push_back(testCaseRedirection308());
	testcases.push_back(testCaseRedirection302());
	testcases.push_back(testCaseRedirection303());
	testcases.push_back(testCaseRedirection307());
	testcases.push_back(testCaseStress());
	testcases.push_back(testCaseVersionNotSupported());
	return testcases;
}
