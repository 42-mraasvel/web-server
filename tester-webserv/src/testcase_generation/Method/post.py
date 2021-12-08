import TestCase
from testcase_generation.Method.default import defaultTestCase

def testCasePostNew():
	# TestCase
	testcase = defaultTestCase()
	testcase.name = "PostNew"
	# Request
	testcase.request.method = 'POST'
	testcase.request.target = '/Post/upload/new.txt'
	with open('./ServerRoot/Method/Post/sample.html', 'rb') as f:
		testcase.request.body = f.read()
	testcase.request.headers['content-length'] = str(len(testcase.request.body))

	# Response
	testcase.response.status_code = 201
	return testcase

def testCasePostOld():
	# TestCase
	testcase = defaultTestCase()
	testcase.name = "PostOld"
	# Request
	testcase.request.method = 'POST'
	testcase.request.target = '/Post/upload/new.txt'
	testcase.request.body = 'Overwriting!!!'
	testcase.request.headers['content-length'] = str(len(testcase.request.body))

	# Response
	testcase.response.status_code = 204
	return testcase
