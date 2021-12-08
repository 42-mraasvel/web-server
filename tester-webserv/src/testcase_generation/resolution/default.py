import TestCase

def defaultTestCase():
	testcase = TestCase.TestCase()
	testcase.tag = 'Resolution'
	testcase.request.headers['Host'] = 'resolution_server'
	return testcase
