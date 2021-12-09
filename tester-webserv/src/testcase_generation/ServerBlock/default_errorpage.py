import TestCase
from testcase_generation.ServerBlock.default import defaultTestCase

def defaultErrorPageTestCase():
	# TestCase
	testcase = defaultTestCase ()
	testcase.request.headers['Host'] = 'serverblock_errorpage'
	return testcase
	
def testCaseDefaultErrorPageRedirect1():
	# TestCase
	testcase = defaultErrorPageTestCase()
	# Request
	testcase.name = "DefaultErrorPageRedirect1"
	testcase.request.method = 'GET'
	testcase.request.target = '/nonexisting.html'
	# Response
	testcase.response.status_code = 404
	testcase.response.expect_body = True
	with open('./ServerRoot/error_pages/404.html', 'rb') as f:
		testcase.response.body = f.read()
	testcase.response.headers['content-length'] = str(len(testcase.response.body))
	return testcase

def testCaseDefaultErrorPageRedirect2():
	# TestCase
	testcase = defaultErrorPageTestCase()
	# Request
	testcase.name = "DefaultErrorPageRedirect2"
	testcase.request.method = 'DELETE'
	testcase.request.target = '/nonexisting.html'

	# Response
	testcase.response.status_code = 405
	testcase.response.expect_body = True
	with open('./ServerRoot/error_pages/405.html', 'rb') as f:
		testcase.response.body = f.read()
	testcase.response.headers['content-length'] = str(len(testcase.response.body))
	testcase.response.headers['allow'] = 'GET, POST'
	
	return testcase

def testCaseDefaultErrorPageRedirectNotFound():
	# TestCase
	testcase = defaultErrorPageTestCase()
	# Request
	testcase.name = 'DefaultErrorPageRedirectNotFound'
	testcase.request.method = 'POST'
	testcase.request.target = '/nonexisting/'
	testcase.request.body = 'Incoming!!!!!'
	testcase.request.headers['content-length'] = str(len(testcase.request.body))

	# Response
	testcase.response.status_code = 400
	testcase.response.expect_body = True
	testcase.response.body = '400 Bad Request\n'
	testcase.response.headers['content-length'] = str(len(testcase.response.body))
	return testcase

def testCaseDefaultErrorPageNotRedirect():
	# TestCase
	testcase = defaultErrorPageTestCase()
	testcase.name = "DefaultErrorPageNotRedirect"
	# Request
	testcase.request.method = 'POST'
	testcase.request.target = '/Method/forbidden/forbidden_dir/nonexisting_dir/nonexisting.html'
	testcase.request.body = 'Incoming!!!'
	testcase.request.headers['content-length'] = str(len(testcase.request.body))
	# Response
	testcase.response.status_code = 403
	testcase.response.expect_body = True
	testcase.response.body = '403 Forbidden\n'
	testcase.response.headers['content-length'] = str(len(testcase.response.body))
	return testcase
