import ParseTestCase
from testcase_generation import autoindex
from testcase_generation import example
from testcase_generation import example2

def generate():
	testcases = ParseTestCase.testCaseFromFiles()
	testcases.append(autoindex.testCaseInit())
	testcases.append(autoindex.testCaseAutoindexOn())
	testcases.append(autoindex.testCaseAutoindexOff())
	testcases.append(autoindex.testCaseAutoindexIndex())
	testcases.append(autoindex.testCaseAutoindexOnPost())
	testcases.append(autoindex.testCaseAutoindexOffDelte())
	testcases.append(example.testCaseFailingTestCase())
	testcases.append(example.testCaseExampleCustomEvaluation())
	testcases.append(example.testCaseSecondTestCase())
	testcases.append(example.testCaseSimpleTestCase())
	testcases.append(example2.testCaseSimpleTestCase())
	return testcases
