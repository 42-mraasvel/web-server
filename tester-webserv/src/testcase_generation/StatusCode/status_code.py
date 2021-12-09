import TestCase
from testcase_generation.StatusCode.default import defaultTestCase

def testCaseExpectError():
	# TestCase
	testcase = defaultTestCase()
	testcase.name = "ExpectError"
	# Request
	testcase.request.method = 'GET'
	testcase.request.target = '/'
	testcase.request.headers['expect'] = 'a100-continue'

	# Response
	testcase.response.status_code = 417
	return testcase

def testCaseMethodNotImplemented():
	# TestCase
	testcase = defaultTestCase()
	testcase.name = "MethodNotImplemented"
	# Request
	testcase.request.method = 'PUT'
	testcase.request.target = '/'

	# Response
	testcase.response.status_code = 501
	return testcase
