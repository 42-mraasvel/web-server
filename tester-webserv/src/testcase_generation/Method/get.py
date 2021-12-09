import TestCase
from testcase_generation.Method.default import defaultTestCase

def testCaseGet():
	# TestCase
	testcase = defaultTestCase()
	testcase.name = "Get"
	# Request
	testcase.request.method = 'GET'
	testcase.request.target = '/Get/sample.html'

	# Response
	testcase.response.status_code = 200
	testcase.response.expect_body = True
	with open('./ServerRoot/Method/Get/sample.html', 'rb') as f:
		testcase.response.body = f.read()
	testcase.response.headers['content-length'] = str(len(testcase.response.body))
	return testcase

def testCaseGetNotFound():
	# TestCase
	testcase = defaultTestCase()
	testcase.name = "GetNotFound"
	# Request
	testcase.request.method = 'GET'
	testcase.request.target = '/Get/nonexisting.html'

	# Response
	testcase.response.status_code = 404
	return testcase


def testCaseGetForbiddenFile():
	# TestCase
	testcase = defaultTestCase()
	testcase.name = "GetForbiddenFile"
	# Request
	testcase.request.method = 'GET'
	testcase.request.target = '/forbidden/forbidden.html'

	# Response
	testcase.response.status_code = 403
	return testcase
