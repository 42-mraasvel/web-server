import Error
import Logger
import requests
from ResponseLogMessage import generateResponseLogMessage

GREEN_BOLD = "\033[32m"
RED_BOLD = "\033[1;31m"
MAGENTA_BOLD = "\033[1;35m"
BLACK_BOLD  ="\033[1;30m"
YELLOW_BOLD ="\033[1;33m"
BLUE_BOLD   ="\033[1;34m"
CYAN_BOLD   ="\033[1;36m"
WHITE_BOLD  ="\033[1;37m"
RESET_COLOR = "\033[0m"

# Expect: list of testcases
def execute(testcases = [], tags = []):
	passed = 0
	failed = 0
	for index, testcase in enumerate(testcases):
		if tags and testcase.tag not in tags:
			continue
		response = sendRequest(testcase.request)
		message = evaluateResponse(response, testcase.response, testcase.evaluator)
		if message is not None:
			failed += 1
			failMsg(message, index, testcase, response)
		else:
			passed += 1
			passMsg(testcase, index)
		response.close()
	endMessage(passed, failed)
	if failed != 0:
		return 1
	return 0

def endMessage(passed, failed):
	print("{}\n ********  Testcases: TOTAL {}{}{} | {}PASS {}{}{} | {}FAIL {}{}{}  ********{}\n".format(MAGENTA_BOLD, RESET_COLOR, passed + failed, MAGENTA_BOLD, GREEN_BOLD, RESET_COLOR, passed, MAGENTA_BOLD, RED_BOLD, RESET_COLOR, failed, MAGENTA_BOLD, RESET_COLOR))

def failMsg(message, index, testcase, response):
	Error.putFail(str(index + 1) + ": [" + testcase.tag + "-" + testcase.name + "]: " + message)
	Logger.log(testcase.getLogString())
	Logger.log(generateResponseLogMessage(response))

def passMsg(testcase, index):
	print(GREEN_BOLD + 'PASS' + RESET_COLOR + ': ' + str(index + 1) + ": [" + testcase.tag + "-" + testcase.name + "]:", \
		testcase.request.method, "with status code:", testcase.response.status_code)

# Send the request
def sendRequest(request):
	uri = 'http://' + request.authority + request.target
	return requests.request(request.method, uri, headers = request.headers, data = request.body)

# Evaluate the response
def evaluateResponse(response, expected, custom_evaluator):
	x = evaluateStatusCode(response.status_code, expected.status_code)
	if x is not None:
		return x
	x = evaluateHeaders(response.headers, expected.headers)
	if x is not None:
		return x
	x = evaluateNoBody(response.status_code, response.content)
	if x is not None:
		return x
	x = evaluateBody(response.content, expected.body, expected.expect_body)
	if x is not None:
		return x
	return custom_evaluator(response)

# evaluateXXX: Should return a tuple (Got, Expected)
# None if everything was as expected
def createMessage(prefix, got, expected):
	return "{}: Got({}), Expected({})".format(prefix, got, expected)

def evaluateStatusCode(status_code, exp_code):
	if exp_code != 0 and status_code != exp_code:
		return createMessage("StatusCode", str(status_code), str(exp_code))
	return None

def evaluateHeaders(headers, exp_headers):
	for key in exp_headers:
		if key not in headers:
			return createMessage('Header', 'NAME_NOT_PRESENT', key)
		elif headers[key] != exp_headers[key]:
			return createMessage('Header', key + ": " + headers[key], key + ": " + exp_headers[key])
	return None

def evaluateNoBody(status_code, content):
	if (status_code >= 300 and status_code < 400) or status_code == 204 or status_code == 304:
		if content:
			return createMessage('Body', 'Message body present', 'No message body')
	return None

def evaluateBody(content, exp_content, should_cmp = True):
	if not should_cmp:
		return None
	if type(exp_content) == str:
		exp_content = exp_content.encode('ascii')
	if content != exp_content:
		# return createMessage('Body', content, exp_content)
		return createMessage('Body', 'Len: ' + str(len(content)), 'Len: ' + str(len(exp_content)))
	return None
