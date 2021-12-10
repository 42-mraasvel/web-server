import TestCase

def defaultTestCase():
	testcase = TestCase.TestCase()
	testcase.tag = 'Response'
	testcase.request.headers['Host'] = 'method'
	return testcase
