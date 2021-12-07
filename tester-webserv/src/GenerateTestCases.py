import ParseTestCase
from testcase_generation import autoindex
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
	testcases.append(example2.testCaseSimpleTestCase())
	testcases.append(example.testCaseFailingTestCase())
	testcases.append(example.testCaseExampleCustomEvaluation())
	testcases.append(example.testCaseSecondTestCase())
	testcases.append(example.testCaseSimpleTestCase())
	return testcases
