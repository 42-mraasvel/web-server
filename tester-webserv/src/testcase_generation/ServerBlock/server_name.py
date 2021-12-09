import TestCase
from testcase_generation.ServerBlock.default import defaultTestCase

def testCaseServerName8080Default():
	# TestCase
	testcase = defaultTestCase()
	testcase.name = "ServerName8080Default"
	# Request
	testcase.request.method = 'GET'
	testcase.request.headers['Host'] = 'nonexisting_server_name'
	testcase.request.target = '/'
	# Response
	testcase.response.status_code = 200
	testcase.response.expect_body = True
	with open('./ServerRoot/ServerBlock/first_server/8080/index.html', 'rb') as f:
		testcase.response.body = f.read()
	testcase.response.headers['content-length'] = str(len(testcase.response.body))
	return testcase

def testCaseServerName8081Default():
	# TestCase
	testcase = defaultTestCase()
	testcase.name = "ServerName8081Default"
	# Request
	testcase.request.authority = 'localhost:8081'
	testcase.request.method = 'GET'
	testcase.request.headers['Host'] = 'nonexisting_server_name'
	testcase.request.target = '/'
	# Response
	testcase.response.status_code = 200
	testcase.response.expect_body = True
	with open('./ServerRoot/ServerBlock/first_server/8081/index.html', 'rb') as f:
		testcase.response.body = f.read()
	testcase.response.headers['content-length'] = str(len(testcase.response.body))
	return testcase

def testCaseServerName8081First():
	# TestCase
	testcase = defaultTestCase()
	testcase.name = "ServerName8081First"
	# Request
	testcase.request.authority = 'localhost:8081'
	testcase.request.method = 'GET'
	testcase.request.headers['Host'] = 'first_server'
	testcase.request.target = '/'
	# Response
	testcase.response.status_code = 200
	testcase.response.expect_body = True
	with open('./ServerRoot/ServerBlock/first_server/8081/index.html', 'rb') as f:
		testcase.response.body = f.read()
	testcase.response.headers['content-length'] = str(len(testcase.response.body))
	return testcase

def testCaseServerName8081Second():
	# TestCase
	testcase = defaultTestCase()
	testcase.name = "ServerName8081Second"
	# Request
	testcase.request.authority = 'localhost:8081'
	testcase.request.method = 'GET'
	testcase.request.headers['Host'] = 'second_server'
	testcase.request.target = '/'
	# Response
	testcase.response.status_code = 405
	testcase.response.expect_body = True
	testcase.response.body = '405 Method Not Allowed\n'
	testcase.response.headers['content-length'] = str(len(testcase.response.body))
	return testcase


def testCaseServerNameWildcardExactMatch():
	# TestCase
	testcase = defaultTestCase()
	testcase.name = "ServerNameWildcardExactMatch"
	# Request
	testcase.request.authority = 'localhost:8081'
	testcase.request.method = 'GET'
	testcase.request.headers['Host'] = 'www.xxx.example.xxx.com'
	testcase.request.target = '/'
	# Response
	testcase.response.status_code = 200
	testcase.response.expect_body = True
	with open('./ServerRoot/ServerBlock/wild_card/exactmatch/index.html', 'rb') as f:
		testcase.response.body = f.read()
	testcase.response.headers['content-length'] = str(len(testcase.response.body))
	return testcase

def testCaseServerNameWildcardFront():
	# TestCase
	testcase = defaultTestCase()
	testcase.name = "ServerNameWildcardFront"
	# Request
	testcase.request.authority = 'localhost:8081'
	testcase.request.method = 'GET'
	testcase.request.headers['Host'] = 'xxx.example.com'
	testcase.request.target = '/'
	# Response
	testcase.response.status_code = 200
	testcase.response.expect_body = True
	with open('./ServerRoot/ServerBlock/wild_card/front/index.html', 'rb') as f:
		testcase.response.body = f.read()
	testcase.response.headers['content-length'] = str(len(testcase.response.body))
	return testcase

def testCaseServerNameWildcardFrontPriority():
	# TestCase
	testcase = defaultTestCase()
	testcase.name = "ServerNameWildcardFrontPriority"
	# Request
	testcase.request.authority = 'localhost:8081'
	testcase.request.method = 'GET'
	testcase.request.headers['Host'] = 'www.example.com'
	testcase.request.target = '/'
	# Response
	testcase.response.status_code = 200
	testcase.response.expect_body = True
	with open('./ServerRoot/ServerBlock/wild_card/front/index.html', 'rb') as f:
		testcase.response.body = f.read()
	testcase.response.headers['content-length'] = str(len(testcase.response.body))
	return testcase

def testCaseServerNameWildcardBack():
	# TestCase
	testcase = defaultTestCase()
	testcase.name = "ServerNameWildcardBack"
	# Request
	testcase.request.authority = 'localhost:8081'
	testcase.request.method = 'GET'
	testcase.request.headers['Host'] = 'www.example.nl'
	testcase.request.target = '/'
	# Response
	testcase.response.status_code = 200
	testcase.response.expect_body = True
	with open('./ServerRoot/ServerBlock/wild_card/back/index.html', 'rb') as f:
		testcase.response.body = f.read()
	testcase.response.headers['content-length'] = str(len(testcase.response.body))
	return testcase
