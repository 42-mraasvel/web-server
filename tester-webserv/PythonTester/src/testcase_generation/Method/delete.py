import TestCase
from testcase_generation.Method.default import defaultTestCase

def testCaseDelete():
	# TestCase
	testcase = defaultTestCase()
	testcase.name = "Delete"
	# Request
	testcase.request.method = 'DELETE'
	testcase.request.target = '/Delete/sample.html'

	# Response
	testcase.response.status_code = 204
	return testcase

def testCaseDeleteDefaultFile():
	# TestCase
	testcase = defaultTestCase()
	testcase.name = "DeleteDefaultFile"
	# Request
	testcase.request.method = 'DELETE'
	testcase.request.target = '/Delete/'

	# Response
	testcase.response.status_code = 400
	return testcase

def testCaseDeleteNotFound():
	# TestCase
	testcase = defaultTestCase()
	testcase.name = "DeleteNotFound"
	# Request
	testcase.request.method = 'DELETE'
	testcase.request.target = '/Delete/nonexisting.html'

	# Response
	testcase.response.status_code = 404
	return testcase

def testCaseDeleteForbiddenFile():
	# TestCase
	testcase = defaultTestCase()
	testcase.name = "DeleteForbiddenFile"
	testcase.tag = "Forbidden"
	# Request
	testcase.request.method = 'DELETE'
	testcase.request.target = '/forbidden/forbidden.html'

	# Response
	testcase.response.status_code = 403
	return testcase

def testCaseDeleteForbiddenDir():
	# TestCase
	testcase = defaultTestCase()
	testcase.name = "DeleteForbiddenDir"
	testcase.tag = "Forbidden"
	# Request
	testcase.request.method = 'DELETE'
	testcase.request.target = '/forbidden/forbidden_dir/sample.html'

	# Response
	testcase.response.status_code = 403
	return testcase
