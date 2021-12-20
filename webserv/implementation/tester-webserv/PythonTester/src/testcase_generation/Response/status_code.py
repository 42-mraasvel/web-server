import TestCase
from testcase_generation.Response.default import defaultTestCase

def testCaseStatusCode415():
	# TestCase
	testcase = defaultTestCase()
	testcase.name = "StatusCode415UnsupportedMedia"
	# Request
	testcase.request.method = 'GET'
	testcase.request.target = '/'
	testcase.request.headers['content-coding'] = 'txt/html'

	# Response
	testcase.response.status_code = 415
	return testcase

def testCaseStatusCode417():
	# TestCase
	testcase = defaultTestCase()
	testcase.name = "StatusCode417ExpectError"
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
	testcase.name = "StatusCode501Method"
	# Request
	testcase.request.method = 'PUT'
	testcase.request.target = '/'

	# Response
	testcase.response.status_code = 501
	return testcase

def testCaseStatusCode501Encoding():
	# TestCase
	testcase = defaultTestCase()
	testcase.name = "StatusCode501Encoding"
	# Request
	testcase.request.method = 'GET'
	testcase.request.target = '/'
	testcase.request.headers['transfer-encoding'] = 'incorrect_encoding'

	# Response
	testcase.response.status_code = 501
	testcase.response.headers['connection'] = 'close'
	return testcase

