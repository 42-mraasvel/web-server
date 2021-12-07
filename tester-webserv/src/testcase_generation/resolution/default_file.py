import TestCase

def defaultTestCase():
	testcase = TestCase.TestCase()
	testcase.tag = 'DefaultFile'
	testcase.request.headers['Host'] = 'default_file_server'
	return testcase

def testCaseDefaultFileGetYes():
	# TestCase
	testcase = defaultTestCase()
	testcase.name = "DefaultFileGetYes"
	# Request
	testcase.request.method = 'GET'
	testcase.request.target = '/'
	# Response
	testcase.response.status_code = 200
	testcase.response.expect_body = True
	with open('./ServerRoot/default_file/index.html', 'rb') as f:
		testcase.response.body = f.read()
	return testcase

def testCaseDefaultFilePost():
	# TestCase
	testcase = defaultTestCase()
	testcase.name = "DefaultFilePost"
	# Request
	testcase.request.method = 'POST'
	testcase.request.target = '/'
	# Response
	testcase.response.status_code = 400
	return testcase

def testCaseDefaultFileDelete():
	# TestCase
	testcase = defaultTestCase()
	testcase.name = "DefaultFileDelete"
	# Request
	testcase.request.method = 'DELETE'
	testcase.request.target = '/'
	# Response
	testcase.response.status_code = 400
	return testcase

def testCaseDefaultFileGetNo():
	# TestCase
	testcase = defaultTestCase()
	testcase.name = "DefaultFileGetNo"
	# Request
	testcase.request.method = 'GET'
	testcase.request.target = '/nonexisting/'
	# Response
	testcase.response.status_code = 404
	return testcase


def testCaseDefaultFileSearch():
	# TestCase
	testcase = defaultTestCase()
	testcase.name = "DefaultFileSearch"
	# Request
	testcase.request.method = 'GET'
	testcase.request.target = '/default_file/'
	# Response
	testcase.response.status_code = 200
	testcase.response.expect_body = True
	with open('./ServerRoot/default_file/index.html', 'rb') as f:
		testcase.response.body = f.read()
	return testcase
