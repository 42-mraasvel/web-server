import sys
import socket
import requests
import parse_request
import TestCase

USAGE_ERR_STR = "Arguments: [Hostname/IP:Port]"
RED_BOLD = "\033[1;31m"
RESET_COLOR = "\033[0m"

def exitError(message):
	assert type(message) == str
	print(RED_BOLD + "ERROR:" + RESET_COLOR, message, file = sys.stderr)
	exit(1)

def simpleTestCase():
	testcase = TestCase.TestCase()

	# Request
	testcase.request.method = 'GET'
	testcase.request.target = '/'
	testcase.request.authority = 'localhost:8080'
	testcase.request.headers['name'] = 'field'
	testcase.request.body = '1234'
	testcase.request.headers['content-length'] = str(len(testcase.request.body))

	# Response
	testcase.response.status_code = 200
	testcase.response.headers['Content-Length'] = '185'
	testcase.response.expect_body = True
	with open('../page_sample/index.html', 'rb') as f:
		testcase.response.body = f.read()
	return testcase

def sendRequest(request):
	uri = 'http://' + request.authority + request.target
	return requests.request(request.method, uri, headers = request.headers, data = request.body)

def evaluateStatusCode(status_code, exp_code):
	if exp_code != 0 and status_code != exp_code:
		return False
	return True

def evaluateHeaders(headers, exp_headers):
	for key in exp_headers:
		if key not in headers:
			return False
		elif headers[key] != exp_headers[key]:
			return False
	return True

def evaluateBody(content, exp_content, should_cmp = True):
	if not should_cmp:
		return True
	return content == exp_content

def evaluateResponse(response, expected):
	return evaluateStatusCode(response.status_code, expected.status_code) \
	and evaluateHeaders(response.headers, expected.headers) \
	and evaluateBody(response.content, expected.body, expected.expect_body)

def executeTestCase(testcase):
	testcase.print()

	response = sendRequest(testcase.request)

	print(response)
	print(response.status_code)
	print(response.headers)
	print(response.content)

	if not evaluateResponse(response, testcase.response):
		print("Invalid response")
	else:
		print("Valid response")
	response.close()


if __name__ == '__main__':
	if len(sys.argv) != 2:
		exitError(USAGE_ERR_STR)

	testcase = simpleTestCase()
	response = executeTestCase(testcase)
