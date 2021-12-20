import TestCase
import Constants

def defaultTestCase():
	testcase = TestCase.TestCase()
	testcase.tag = 'LocationBlock'
	testcase.request.method = 'GET'
	testcase.request.headers['Host'] = 'locationblock'
	return testcase

def testCaseResolutionMatchSlash():
	testcase = defaultTestCase()

	testcase.name = 'ResolutionMatchSlash'
	# Request
	testcase.request.target = '/'
	# Response
	testcase.response.status_code = 200
	testcase.response.expect_body = True
	with open(Constants.SERVER_ROOT + '/LocationBlock/index.html', 'rb') as f:
		testcase.response.body = f.read()
	testcase.response.headers['content-length'] = str(len(testcase.response.body))
	return testcase

def testCaseResolutionBlockOne():
	testcase = defaultTestCase()
	testcase.name = 'ResolutionBlockOne'
	# Request
	testcase.request.target = '/block1/'
	# Response
	testcase.response.status_code = 200
	testcase.response.expect_body = True
	with open(Constants.SERVER_ROOT + '/LocationBlock/block1/block1/index.html', 'rb') as f:
		testcase.response.body = f.read()
	testcase.response.headers['content-length'] = str(len(testcase.response.body))
	return testcase

def testCaseResolutionBlockTwo():
	testcase = defaultTestCase()
	testcase.name = 'ResolutionExactMatch'
	testcase.request.target = '/block2/index.html'
	# Response
	testcase.response.status_code = 200
	testcase.response.expect_body = True
	with open(Constants.SERVER_ROOT + '/LocationBlock/block2/block2/index.html', 'rb') as f:
		testcase.response.body = f.read()
	testcase.response.headers['content-length'] = str(len(testcase.response.body))
	return testcase

def testCaseResolutionNonExistantBlock():
	testcase = defaultTestCase()
	testcase.name = 'ResolutionNoMatch'
	testcase.request.target = '/doesntexist/x/y/z'
	# Response
	testcase.response.status_code = 404
	return testcase
