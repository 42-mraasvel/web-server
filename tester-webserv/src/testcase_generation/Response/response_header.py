import TestCase
from testcase_generation.Response.default import defaultTestCase

def testCaseConnectionClose():
	# TestCase
	testcase = defaultTestCase()
	testcase.name = "ConnectionClose"
	# Request
	testcase.request.method = 'GET'
	testcase.request.target = '/Get/sample.html'
	testcase.request.headers['connection'] = 'close'

	# Response
	testcase.response.status_code = 200
	testcase.response.expect_body = True
	with open('./ServerRoot/Method//Get/sample.html', 'rb') as f:
		testcase.response.body = f.read()
	testcase.response.headers['content-length'] = str(len(testcase.response.body))
	testcase.response.headers['connection'] = 'close'
	return testcase
