import TestCase

def testCaseInit():
	# Request
	request = TestCase.Request()
	# Response
	response = TestCase.Response()
	return TestCase.TestCase(request, response)

def testCaseAutoindexOn():
	testcase = testCaseInit()
	# TestCase
	testcase.tag = "AutoIndex"
	testcase.name = "AutoIndexOn"
	# Request
	testcase.request.method = 'GET'
	testcase.request.target = '/auto_index/on/'
	# Response
	testcase.response.status_code = 200
	testcase.response.expect_body = True
	with open('../page_sample/auto_index/index.html', 'rb') as f:
		testcase.response.body = f.read()
	return testcase

def testCaseAutoindexOff():
	testcase = testCaseInit()
	# TestCase
	testcase.tag = "AutoIndex"
	testcase.name = "AutoIndexOn"
	# Request
	testcase.request.method = 'GET'
	testcase.request.target = '/auto_index/off/'
	# Response
	testcase.response.status_code = 404
	return testcase

def testCaseAutoindexIndex():
	testcase = testCaseInit()
	# TestCase
	testcase.tag = "AutoIndex"
	testcase.name = "AutoIndexOff"
	# Request
	testcase.request.method = 'GET'
	testcase.request.target = '/auto_index/index/'
	# Response
	testcase.response.status_code = 200
	testcase.response.expect_body = True
	with open('../page_sample/auto_index/index/index.html', 'rb') as f:
		testcase.response.body = f.read()
	return testcase

def testCaseAutoindexOnPost():
	testcase = testCaseInit()
	# TestCase
	testcase.tag = "AutoIndex"
	testcase.name = "AutoIndexPost"
	# Request
	testcase.request.method = 'POST'
	testcase.request.target = '/auto_index/on/'
	# Response
	testcase.response.status_code = 404
	return testcase

def testCaseAutoindexOffDelte():
	testcase = testCaseInit()
	testcase.name = "AutoIndexDelete"
	# TestCase
	testcase.tag = "AutoIndex"
	# Request
	testcase.request.method = 'DELETE'
	testcase.request.target = '/auto_index/off/'
	# Response
	testcase.response.status_code = 404
	return testcase
