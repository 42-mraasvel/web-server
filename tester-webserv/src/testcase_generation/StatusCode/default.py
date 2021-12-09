import TestCase

def defaultTestCase():
	testcase = TestCase.TestCase()
	testcase.tag = 'StatusCode'
	testcase.request.headers['Host'] = 'method'
	return testcase
