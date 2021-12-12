import TestCase

def defaultCase():
	testcase = TestCase.TestCase()

	testcase.tag = 'CGI'
	testcase.request.headers['Host'] = 'CgiBlock'
	return testcase

def testCase42Get():
	testcase = defaultCase()

	testcase.name = '42Get'
	# Request
	testcase.request.method = 'GET'
	testcase.request.target = '/example.bla'

	# Response
	testcase.response.status_code = 200
	return testcase

def testCase42Post():
	testcase = defaultCase()

	testcase.name = '42Post'
	# Request
	testcase.request.method = 'POST'
	testcase.request.target = '/posting.bla'
	# testcase.request.body = '1234'

	# Response
	testcase.response.status_code = 200
	return testcase
