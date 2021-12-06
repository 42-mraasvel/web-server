import TestCase
import ParseTestCase
from TestCaseGeneration import test

#
# MAIN GENERATE FUNCTION
# Append new TestCases to the `testcases` list to execute them
#
def generate():
	testcases = ParseTestCase.testCaseFromFiles()

	testcases.append(simpleTestCase())
	testcases.append(secondTestCase())
	testcases.append(failingTestCase())
	testcases.append(test.testFunction())
	testcases.append(exampleCustomEvaluation())
	return testcases

#
# Example testcases
#
def failingTestCase():
	testcase = simpleTestCase()
	testcase.tag = "FailingTag"
	testcase.response.status_code = 201
	return testcase

def failingEvaluation(response):
	return "Custom evaluation failed string"
	return None # Return None to validate the response

def exampleCustomEvaluation():
	testcase = simpleTestCase()
	testcase.evaluator = failingEvaluation
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

	return TestCase.TestCase(request, response)