import GenerateTestCases

def testFunction():
	testcase = GenerateTestCases.simpleTestCase()
	testcase.tag = "FailingTag"
	testcase.response.status_code = 201
	return testcase
