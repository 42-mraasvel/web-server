import TestCase
from testcase_generation.Response.default import defaultTestCase

def testCaseStatusCode417():
	# TestCase
	testcase = defaultTestCase()
	testcase.name = "StatusCodeExpectError"
	# Request
	testcase.request.method = 'GET'
	testcase.request.target = '/'
	testcase.request.headers['expect'] = 'a100-continue'

	# Response
	testcase.response.status_code = 417
	return testcase

def testCaseStatusCode501Method():
	# TestCase
	testcase = defaultTestCase()
	testcase.name = "MethodNotImplementedMethod"
	# Request
	testcase.request.method = 'PUT'
	testcase.request.target = '/'

	# Response
	testcase.response.status_code = 501
	return testcase

def testCaseStatusCode501Encoding():
	# TestCase
	testcase = defaultTestCase()
	testcase.name = "MethodNotImplementedEncoding"
	# Request
	testcase.request.method = 'GET'
	testcase.request.target = '/'
	testcase.request.headers['transfer-encoding'] = 'incorrect_encoding'

	# Response
	testcase.response.status_code = 501
	testcase.response.headers['connection'] = 'close'
	return testcase

