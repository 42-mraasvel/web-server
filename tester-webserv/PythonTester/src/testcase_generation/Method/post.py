import TestCase
from testcase_generation.Method.default import defaultTestCase
import Constants

def testCasePostUploadDirNew():
	# TestCase
	testcase = defaultTestCase()
	testcase.name = "PostUploadDirNew"
	# Request
	testcase.request.method = 'POST'
	testcase.request.target = '/Post/upload/new.txt'
	with open(Constants.SERVER_ROOT + '/Method/Get/sample.html', 'rb') as f:
		testcase.request.body = f.read()
	testcase.request.headers['content-length'] = str(len(testcase.request.body))

	# Response
	testcase.response.status_code = 201
	testcase.response.headers['location'] = str(testcase.request.target)
	return testcase

def testCasePostUploadDirOld():
	# TestCase
	testcase = defaultTestCase()
	testcase.name = "PostUploadDirOld"
	# Request
	testcase.request.method = 'POST'
	testcase.request.target = '/Post/upload/new.txt'
	testcase.request.body = 'Overwriting!!!'
	testcase.request.headers['content-length'] = str(len(testcase.request.body))
	# Response
	testcase.response.status_code = 204
	return testcase

def testCasePostRootDirNew():
	# TestCase
	testcase = defaultTestCase()
	testcase.name = "PostRootDirNew"
	# Request
	testcase.request.method = 'POST'
	testcase.request.target = '/Post/root/new.txt'
	with open(Constants.SERVER_ROOT + '/Method/Get/sample.html', 'rb') as f:
		testcase.request.body = f.read()
	testcase.request.headers['content-length'] = str(len(testcase.request.body))

	# Response
	testcase.response.status_code = 201
	testcase.response.headers['location'] = str(testcase.request.target)
	return testcase

def testCasePostRootDirOld():
	# TestCase
	testcase = defaultTestCase()
	testcase.name = "PostRootDirOld"
	# Request
	testcase.request.method = 'POST'
	testcase.request.target = '/Post/root/new.txt'
	testcase.request.body = 'Overwriting!!!'
	testcase.request.headers['content-length'] = str(len(testcase.request.body))

	# Response
	testcase.response.status_code = 204
	return testcase

def testCasePostForbiddenFile():
	# TestCase
	testcase = defaultTestCase()
	testcase.name = "PostUploadDirForbiddenFile"
	testcase.tag = "Forbidden"
	# Request
	testcase.request.method = 'POST'
	testcase.request.target = '/forbidden/forbidden.html'
	testcase.request.body = 'Incoming!!!'
	testcase.request.headers['content-length'] = str(len(testcase.request.body))
	# Response
	testcase.response.status_code = 403
	return testcase

def testCasePostForbiddenDir1():
	# TestCase
	testcase = defaultTestCase()
	testcase.name = "PostUploadDirForbiddenDir1"
	testcase.tag = "Forbidden"
	# Request
	testcase.request.method = 'POST'
	testcase.request.target = '/forbidden/forbidden_dir/sample.html'
	testcase.request.body = 'Incoming!!!'
	testcase.request.headers['content-length'] = str(len(testcase.request.body))
	# Response
	testcase.response.status_code = 403
	return testcase

def testCasePostForbiddenDir2():
	# TestCase
	testcase = defaultTestCase()
	testcase.name = "PostUploadDirForbiddenDir2"
	testcase.tag = "Forbidden"
	# Request
	testcase.request.method = 'POST'
	testcase.request.target = '/forbidden/forbidden_dir/nonexisting_dir/nonexisting.html'
	testcase.request.body = 'Incoming!!!'
	testcase.request.headers['content-length'] = str(len(testcase.request.body))
	# Response
	testcase.response.status_code = 403
	return testcase
