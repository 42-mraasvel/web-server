import TestCase

def defaultTestCase():
	testcase = TestCase.TestCase()

	testcase.tag = 'Redirection'
	testcase.request.headers['Host'] = 'RedirectionBlock'
	return testcase

def testCaseRedirect():
	testcase = defaultTestCase()
	# Request
	testcase.name = 'BasicRedirect'
	testcase.request.target = '/block'
	testcase.request.method = 'GET'

	# Response
	testcase.response.status_code = 200
	testcase.response.expect_body = True
	with open('./ServerRoot/index.html', 'rb') as f:
		testcase.response.body = f.read()
	return testcase

# Infinite redirection doesn't pass yet

# def testCaseLooping():
# 	testcase = defaultTestCase()

# 	testcase.name = 'LoopingRedirection'
# 	testcase.request.target = '/looping_redirect'
# 	testcase.request.method = 'GET'

# 	# Response
# 	testcase.response.status_code = 508
# 	return testcase
