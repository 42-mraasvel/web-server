import TestCase
from testcase_generation.ServerBlock.default import defaultTestCase

def testCaseServerNameNotFound():
	# TestCase
	testcase = defaultTestCase()
	testcase.tag = 'temp'
	# Request
	testcase.name = "ServerNameNotFound"
	testcase.request.method = 'GET'
	testcase.request.headers['Host'] = 'nonexisting_server_name'
	testcase.request.target = '/'
	# Response
	testcase.response.status_code = 200
	testcase.response.expect_body = True
	with open('./ServerRoot/FirstServer/index.html', 'rb') as f:
		testcase.response.body = f.read()
	testcase.response.headers['content-length'] = str(len(testcase.response.body))
	return testcase
