import TestCase
from testcase_generation.Location_Block.resolution import defaultTestCase

def testCaseGetNotAllowed():
	testcase = defaultTestCase()
	testcase.name = 'GetNotAllowed'
	testcase.request.target = '/GET_NOT_ALLOWED'

	testcase.response.status_code = 405
	testcase.response.headers['Allow'] = 'POST, DELETE'
	return testcase

def testCasePostNotAllowed():
	testcase = defaultTestCase()
	testcase.name = 'PostNotAllowed'

	testcase.request.method = 'POST'
	testcase.request.target = '/NOT_ALLOWED'

	testcase.response.status_code = 405
	testcase.response.headers['Allow'] = 'GET'
	return testcase

def testCaseDeleteNotAllowed():
	testcase = defaultTestCase()
	testcase.name = 'DeleteNotAllowed'
	testcase.request.method = 'DELETE'
	testcase.request.target = '/NOT_ALLOWED'

	testcase.response.status_code = 405
	testcase.response.headers['Allow'] = 'GET'
	return testcase
