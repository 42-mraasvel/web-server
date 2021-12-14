#include "TestCase.hpp"

TestCase testCaseStress0();
TestCase testCaseVersionNotSupported();
TestCase testCaseRedirection301();
TestCase testCaseRedirection308();
TestCase testCaseRedirection302();
TestCase testCaseRedirection303();
TestCase testCaseRedirection307();
TestCase testCaseBadRequestMethod();
TestCase testCaseBadRequestTarget();
TestCase testCaseBadRequestVersion();
TestCase testCaseBadRequestFirstLineDelimeter();
TestCase testCaseBadRequestURITooLong();
TestCase testCaseBadRequestHost();
TestCase testCaseBadRequestConnection();
TestCase testCaseBadRequestContentLength();
TestCase testCaseBadRequestHeader();
TestCase testCaseExpect();
TestCase testCaseCgi();
TestCase testCaseConnectionDefault0();
TestCase testCaseConnectionKeepAlive();
TestCase testCaseConnectionClose();
TestCase testCaseConnectionDefault1();
TestCase testCaseUnsafe();
TestCase testCaseTimeout();
TestCase testCaseTimeoutWithRequest();

TestCaseVector generateTestCases() {
	TestCaseVector testcases;
	testcases.push_back(testCaseStress0());
	testcases.push_back(testCaseVersionNotSupported());
	testcases.push_back(testCaseRedirection301());
	testcases.push_back(testCaseRedirection308());
	testcases.push_back(testCaseRedirection302());
	testcases.push_back(testCaseRedirection303());
	testcases.push_back(testCaseRedirection307());
	testcases.push_back(testCaseBadRequestMethod());
	testcases.push_back(testCaseBadRequestTarget());
	testcases.push_back(testCaseBadRequestVersion());
	testcases.push_back(testCaseBadRequestFirstLineDelimeter());
	testcases.push_back(testCaseBadRequestURITooLong());
	testcases.push_back(testCaseBadRequestHost());
	testcases.push_back(testCaseBadRequestConnection());
	testcases.push_back(testCaseBadRequestContentLength());
	testcases.push_back(testCaseBadRequestHeader());
	testcases.push_back(testCaseExpect());
	testcases.push_back(testCaseCgi());
	testcases.push_back(testCaseConnectionDefault0());
	testcases.push_back(testCaseConnectionKeepAlive());
	testcases.push_back(testCaseConnectionClose());
	testcases.push_back(testCaseConnectionDefault1());
	testcases.push_back(testCaseUnsafe());
	testcases.push_back(testCaseTimeout());
	testcases.push_back(testCaseTimeoutWithRequest());
	return testcases;
}
