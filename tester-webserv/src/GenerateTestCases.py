import TestCase

def generate():
	testcases = []

	testcases.append(simpleTestCase())
	testcases.append(secondTestCase())
	testcases.append(invalidTestCase())
	testcases.append(invalidTestCase())
	testcases.append(invalidTestCase())
	return testcases

def invalidTestCase():
	testcase = simpleTestCase()
	testcase.response.status_code = 201
	return testcase

def secondTestCase():
	testcase = simpleTestCase()
	testcase.tag = 'Second'
	testcase.request.body = 'Test'
	testcase.request.headers['REQUEST'] = 'SECOND'
	return testcase

def simpleTestCase():

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

	tag = "First"
	return TestCase.TestCase(request, response, tag)