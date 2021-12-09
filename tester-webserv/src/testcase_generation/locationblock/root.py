import TestCase

def defaultTestCase():
	testcase = TestCase.TestCase()
	testcase.tag = 'locationblock'
	testcase.request.method = 'GET'
	testcase.request.headers['Host'] = 'locationblock_root'
	testcase.response.status_code = 404
	return testcase

def testCaseRootEmpty():
	testcase = defaultTestCase()
	# Request
	testcase.name = 'RootEmpty'
	testcase.request.target = '/emptyroot'
	return testcase

def testCaseRootError1():
	testcase = defaultTestCase()
	# Request
	testcase.name = 'RootError1'
	testcase.request.target = '/error1'
	return testcase

def testCaseRootError2():
	testcase = defaultTestCase()
	# Request
	testcase.name = 'RootError2'
	testcase.request.target = '/error2'
	return testcase

def testCaseRootError3():
	testcase = defaultTestCase()
	# Request
	testcase.name = 'RootError3'
	testcase.request.target = '/error3'
	return testcase

def testCaseRootError4():
	testcase = defaultTestCase()
	# Request
	testcase.name = 'RootError4'
	testcase.request.target = '/error4'
	return testcase

def testCaseRootError5():
	testcase = defaultTestCase()
	# Request
	testcase.name = 'RootError5'
	testcase.request.target = '/error5'
	return testcase
