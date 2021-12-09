import TestCase

def defaultTestCase():
	testcase = TestCase.TestCase()
	testcase.tag = 'Method'
	testcase.request.headers['Host'] = 'method'
	return testcase
