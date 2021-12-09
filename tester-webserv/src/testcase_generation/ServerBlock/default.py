import TestCase

def defaultTestCase():
	testcase = TestCase.TestCase()
	testcase.tag = 'ServerBlock'
	testcase.request.headers['Host'] = 'serverblock'
	return testcase
