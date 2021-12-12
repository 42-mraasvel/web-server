import TestCase

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
	response.headers['content-length'] = '185'
	response.expect_body = True
	with open('../page_sample/index.html', 'rb') as f:
		response.body = f.read()

	return TestCase.TestCase(request, response)
