import TestCase

def defaultTestCase():
	testcase = TestCase.TestCase()
	testcase.tag = 'LocationBlock'
	testcase.request.method = 'GET'
	testcase.request.headers['Host'] = 'ServerBlock'
	return testcase

def testCaseMatchSlash():
	testcase = defaultTestCase()

	testcase.name = 'MatchSlash'
	# Request
	testcase.request.target = '/'
	# Response
	testcase.response.status_code = 200
	with open('./ServerRoot/ServerBlock/index.html', 'rb') as f:
		testcase.response.body = f.read()
	testcase.response.expect_body = True
	return testcase

def testCaseBlockOne():
	testcase = defaultTestCase()
	testcase.name = 'BlockOne'
	# Request
	testcase.request.target = '/block1/'
	# Response
	testcase.response.status_code = 200
	with open('./ServerRoot/ServerBlock/block1/block1/index.html', 'rb') as f:
		testcase.response.body = f.read()
	testcase.response.expect_body = True
	return testcase

def testCaseBlockTwo():
	testcase = defaultTestCase()
	testcase.name = 'ExactMatch'
	testcase.request.target = '/block2/index.html'
	# Response
	testcase.response.status_code = 200
	with open('./ServerRoot/ServerBlock/block2/block2/index.html', 'rb') as f:
		testcase.response.body = f.read()
	testcase.response.expect_body = True
	return testcase

def testCaseNonExistantBlock():
	testcase = defaultTestCase()
	testcase.name = 'NoMatch'
	testcase.request.target = '/doesntexist/x/y/z'
	# Response
	testcase.response.status_code = 404
	return testcase
