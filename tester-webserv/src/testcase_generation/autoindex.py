import TestCase

def defaultTestCase():
	testcase = TestCase.TestCase()
	testcase.tag = 'AutoIndex'
	testcase.request.headers['Host'] = 'autoindex_server'
	return testcase

def testCaseAutoindexOnGet():
	# TestCase
	testcase = defaultTestCase()
	testcase.name = "AutoindexOnGet"
	# Request
	testcase.request.method = 'GET'
	testcase.request.target = '/on/'
	# Response
	testcase.response.status_code = 200
	testcase.response.expect_body = True
	with open('./ServerRoot/auto_index/index.html', 'rb') as f:
		testcase.response.body = f.read()
	return testcase

def testCaseAutoindexOnPost():
	# TestCase
	testcase = defaultTestCase()
	testcase.name = "AutoindexOnPost"
	# Request
	testcase.request.method = 'POST'
	testcase.request.target = '/on/'
	# Response
	testcase.response.status_code = 400
	return testcase

def testCaseAutoindexOnDelete():
	# TestCase
	testcase = defaultTestCase()
	testcase.name = "AutoindexOnDelete"
	# Request
	testcase.request.method = 'DELETE'
	testcase.request.target = '/on/'
	# Response
	testcase.response.status_code = 400
	return testcase

def testCaseAutoindexOffGet():
	# TestCase
	testcase = defaultTestCase()
	testcase.name = "AutoindexOffGet"
	# Request
	testcase.request.method = 'GET'
	testcase.request.target = '/off/'
	# Response
	testcase.response.status_code = 404
	return testcase

def testCaseAutoindexOffPost():
	# TestCase
	testcase = defaultTestCase()
	testcase.name = "AutoindexOffPost"
	# Request
	testcase.request.method = 'POST'
	testcase.request.target = '/off/'
	# Response
	testcase.response.status_code = 404
	return testcase

def testCaseAutoindexOffDelete():
	# TestCase
	testcase = defaultTestCase()
	testcase.name = "AutoindexOffDelete"
	# Request
	testcase.request.method = 'DELETE'
	testcase.request.target = '/off/'
	# Response
	testcase.response.status_code = 404
	return testcase

def testCaseAutoindexIndexGet():
	# TestCase
	testcase = defaultTestCase()
	testcase.name = "AutoindexIndexGet"
	# Request
	testcase.request.method = 'GET'
	testcase.request.target = '/index/'
	# Response
	testcase.response.status_code = 200
	testcase.response.expect_body = True
	with open('./ServerRoot/auto_index/index/index.html', 'rb') as f:
		testcase.response.body = f.read()
	return testcase

def testCaseAutoindexIndexPost():
	# TestCase
	testcase = defaultTestCase()
	testcase.name = "AutoindexIndexPost"
	# Request
	testcase.request.method = 'POST'
	testcase.request.target = '/index/'
	# Response
	testcase.response.status_code = 400
	return testcase

def testCaseAutoindexIndexDelete():
	# TestCase
	testcase = defaultTestCase()
	testcase.name = "AutoindexIndexDelete"
	# Request
	testcase.request.method = 'DELETE'
	testcase.request.target = '/index/'
	# Response
	testcase.response.status_code = 400
	return testcase
