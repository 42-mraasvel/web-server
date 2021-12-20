import TestCase
import Constants

def defaultTestCase():
	testcase = TestCase.TestCase()
	testcase.tag = 'LocationBlock'
	testcase.request.headers['Host'] = 'locationblock'
	return testcase

def testCaseRedirectGet():
	testcase = defaultTestCase()
	# Request
	testcase.name = 'BasicRedirectGet'
	testcase.request.target = '/redirection'
	testcase.request.method = 'GET'

	# Response
	testcase.response.status_code = 200
	testcase.response.expect_body = True
	with open(Constants.SERVER_ROOT + '/LocationBlock/index.html', 'rb') as f:
		testcase.response.body = f.read()
	testcase.response.headers['content-length'] = str(len(testcase.response.body))
	return testcase

def testCaseRedirectPost():
	testcase = defaultTestCase()
	# Request
	testcase.name = 'BasicRedirectPost'
	testcase.request.target = '/redirection'
	testcase.request.method = 'POST'
	testcase.request.body = 'Overwriting!!!'
	testcase.request.headers['content-length'] = str(len(testcase.request.body))

	# Response
	testcase.response.status_code = 200
	testcase.response.expect_body = True
	with open(Constants.SERVER_ROOT + '/LocationBlock/index.html', 'rb') as f:
		testcase.response.body = f.read()
	testcase.response.headers['content-length'] = str(len(testcase.response.body))
	return testcase

def testCaseRedirectDelete():
	testcase = defaultTestCase()
	# Request
	testcase.name = 'BasicRedirectDelete'
	testcase.request.target = '/redirection/delete'
	testcase.request.method = 'DELETE'

	# Response
	testcase.response.status_code = 404
#	testcase.response.expect_body = True
#	with open(Constants.SERVER_ROOT + '/LocationBlock/index.html', 'rb') as f:
#		testcase.response.body = f.read()
#	testcase.response.headers['content-length'] = str(len(testcase.response.body))
	return testcase

# Infinite redirection doesn't pass yet

# def testCaseLooping():
# 	testcase = defaultTestCase()

# 	testcase.name = 'LoopingRedirection'
# 	testcase.request.target = '/redirection/looping_redirect'
# 	testcase.request.method = 'GET'

# 	# Response
# 	testcase.response.status_code = 508
# 	return testcase
