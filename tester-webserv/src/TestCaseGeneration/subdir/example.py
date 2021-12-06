#
# Example testcases
#
def testCaseFailingTestCase():
	testcase = simpleTestCase()
	testcase.tag = "FailingTag"
	testcase.response.status_code = 201
	return testcase

def testCaseFailingEvaluation(response):
	return "Custom evaluation failed string"
	return None # Return None to validate the response

def testCaseExampleCustomEvaluation():
	testcase = simpleTestCase()
	testcase.evaluator = failingEvaluation
	return testcase

def testCaseSecondTestCase():
	testcase = simpleTestCase()
	testcase.tag = 'Second'
	testcase.request.body = 'Test'
	testcase.request.headers['REQUEST'] = 'SECOND'
	return testcase

def testCaseSimpleTestCase():

	# Request
	request = TestCase.Request()
	request.method = 'GET'
	request.target = '/'
	request.headers['name'] = 'field'
	request.headers['REQUEST'] = 'FIRST'
	request.headers['expect'] = '100-continue'
	request.body = '1234'
	request.headers['content-length'] = str(len(request.body))

	# Response
	response = TestCase.Response()
	response.status_code = 200
	response.headers['Content-Length'] = '185'
	response.expect_body = True
	with open('../page_sample/index.html', 'rb') as f:
		response.body = f.read()

	return TestCase.TestCase(request, response)
