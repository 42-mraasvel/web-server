import TestCase
from testcase_generation.Response.default import defaultTestCase

def defaultMediaTypeTestCase():
	testcase = defaultTestCase()
	# Request
	testcase.request.method = 'GET'
	# Response
	testcase.response.status_code = 200
	testcase.response.expect_body = True
	return testcase
	
def testCaseMediaType_html():
	testcase = defaultMediaTypeTestCase()
	testcase.name = 'MediaType_html'
	testcase.request.target = '/MediaType/sample.html'
	testcase.response.headers['Content-Type'] = 'text/html'
	with open('./ServerRoot/Method/MediaType/sample.html', 'rb') as f:
		testcase.response.body = f.read()
	testcase.response.headers['content-length'] = str(len(testcase.response.body))
	return testcase

def testCaseMediaType_txt():
	testcase = defaultMediaTypeTestCase()
	testcase.name = 'MediaType_txt'
	testcase.request.target = '/MediaType/sample.txt'
	testcase.response.headers['Content-Type'] = 'text/plain;charset=UTF-8'
	with open('./ServerRoot/Method/MediaType/sample.txt', 'rb') as f:
		testcase.response.body = f.read()
	testcase.response.headers['content-length'] = str(len(testcase.response.body))
	return testcase

def testCaseMediaType_png():
	testcase = defaultMediaTypeTestCase()
	testcase.name = 'MediaType_png'
	testcase.request.target = '/MediaType/sample.png'
	testcase.response.headers['Content-Type'] = 'image/png'
	with open('./ServerRoot/Method/MediaType/sample.png', 'rb') as f:
		testcase.response.body = f.read()
	testcase.response.headers['content-length'] = str(len(testcase.response.body))
	return testcase

def testCaseMediaType_jpeg():
	testcase = defaultMediaTypeTestCase()
	testcase.name = 'MediaType_png'
	testcase.request.target = '/MediaType/sample.jpeg'
	testcase.response.headers['Content-Type'] = 'image/jpeg'
	with open('./ServerRoot/Method/MediaType/sample.jpeg', 'rb') as f:
		testcase.response.body = f.read()
	testcase.response.headers['content-length'] = str(len(testcase.response.body))
	return testcase

def testCaseMediaType_sh():
	testcase = defaultMediaTypeTestCase()
	testcase.name = 'MediaType_sh'
	testcase.request.target = '/MediaType/sample.sh'
	testcase.response.headers['Content-Type'] = 'application/x-sh'
	with open('./ServerRoot/Method/MediaType/sample.sh', 'rb') as f:
		testcase.response.body = f.read()
	testcase.response.headers['content-length'] = str(len(testcase.response.body))
	return testcase
