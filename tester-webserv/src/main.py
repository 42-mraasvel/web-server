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

def SimpleTestCase():
	testcase = TestCase.TestCase()

	# Request
	testcase.request.method = 'GET'
	testcase.request.request_target = '/'
	testcase.request.authority = 'localhost:8080'
	testcase.request.header['name'] = 'field'
	testcase.request.body = '1234'
	testcase.request.header['content-length'] = len(testcase.request.body)

	# Response
	testcase.response.status_code = 200
	return testcase

def ExecuteTestCase(testcase):
	testcase.print()


if __name__ == '__main__':
	if len(sys.argv) != 2:
		exitError(USAGE_ERR_STR)

	testcase = SimpleTestCase()
	ExecuteTestCase(testcase)