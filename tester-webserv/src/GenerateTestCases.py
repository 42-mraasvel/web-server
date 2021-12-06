import ParseTestCase
from testcase_generation import example
from testcase_generation import example2

def generate():
	testcases = ParseTestCase.testCaseFromFiles()
	testcases.append(example.testCaseFailingTestCase())
	testcases.append(example.testCaseExampleCustomEvaluation())
	testcases.append(example.testCaseSecondTestCase())
	testcases.append(example.testCaseSimpleTestCase())
	testcases.append(example2.testCaseSimpleTestCase())
	return testcases
