import TestCase
from testcase_generation.ServerBlock.default import defaultTestCase

def testCaseMaxBodySizeGet():
	# TestCase
	testcase = defaultTestCase()
	# Request
	testcase.request.headers['Host'] = 'max_body_size'
	testcase.request.target = '/nonexisting.html'
	testcase.request.body = '123456789012345678901234567890123456789012345678901234567890'
	testcase.request.headers['content-length'] = str(len(testcase.request.body))
	testcase.name = "MaxBodySizeGet"
	testcase.request.method = 'GET'
	# Response
	testcase.response.status_code = 413
	return testcase

def testCaseMaxBodySizePost():
	# TestCase
	testcase = testCaseMaxBodySizeGet()
	testcase.name = "MaxBodySizePost"
	# Request
	testcase.request.method = 'POST'
	return testcase

def testCaseMaxBodySizeDelete():
	# TestCase
	testcase = testCaseMaxBodySizeGet()
	testcase.name = "MaxBodySizeDelete"
	# Request
	testcase.request.method = 'DELETE'
	return testcase
