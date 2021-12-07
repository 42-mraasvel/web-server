import ParseTestCase
from testcase_generation import autoindex
from testcase_generation.locationblock import resolution
from testcase_generation.locationblock import notallowed
from testcase_generation.locationblock import redirection
from testcase_generation.cgi import tester42
from testcase_generation import example2
from testcase_generation import example

def generate():
	testcases = ParseTestCase.testCaseFromFiles()
	testcases.append(autoindex.testCaseInit())
	testcases.append(autoindex.testCaseAutoindexOn())
	testcases.append(autoindex.testCaseAutoindexOff())
	testcases.append(autoindex.testCaseAutoindexIndex())
	testcases.append(autoindex.testCaseAutoindexOnPost())
	testcases.append(autoindex.testCaseAutoindexOffDelte())
	testcases.append(resolution.testCaseMatchSlash())
	testcases.append(resolution.testCaseBlockOne())
	testcases.append(resolution.testCaseBlockTwo())
	testcases.append(resolution.testCaseNonExistantBlock())
	testcases.append(notallowed.testCaseGetNotAllowed())
	testcases.append(notallowed.testCasePostNotAllowed())
	testcases.append(notallowed.testCaseDeleteNotAllowed())
	testcases.append(redirection.testCaseRedirect())
	testcases.append(tester42.testCase42Get())
	testcases.append(tester42.testCase42Post())
	testcases.append(example2.testCaseSimpleTestCase())
	testcases.append(example.testCaseFailingTestCase())
	testcases.append(example.testCaseExampleCustomEvaluation())
	testcases.append(example.testCaseSecondTestCase())
	testcases.append(example.testCaseSimpleTestCase())
	return testcases
