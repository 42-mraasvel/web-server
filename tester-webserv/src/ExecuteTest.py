import Error
import Logger
import requests

GREEN_BOLD = "\033[32m"
RESET_COLOR = "\033[0m"

# Expect: list of testcases
def execute(testcases = [], authority = 'localhost:8080', tags = []):
	passed = 0
	failed = 0
	for index, testcase in enumerate(testcases):
		if tags and testcase.tag not in tags:
			continue
		response = sendRequest(testcase.request, authority)
		message = evaluateResponse(response, testcase.response, testcase.evaluator)
		if message is not None:
			failed += 1
			failMsg(message, index, testcase)
		else:
			passed += 1
			passMsg(testcase, index)
		response.close()
	endMessage(passed, failed)

def endMessage(passed, failed):
	print("Executed: {} testcases: PASS({}) PASS({})".format(passed + failed, passed, failed))

def failMsg(message, index, testcase):
	Error.putFail(str(index + 1) + ": [" + testcase.tag + "]: " + message)
	Logger.log(testcase.getLogString())

def passMsg(testcase, index):
	print(GREEN_BOLD + 'PASS' + RESET_COLOR + ': ' + str(index + 1) + ": [" + testcase.tag + "]:", \
		testcase.request.method, "with status code:", testcase.response.status_code)

# Send the request
def sendRequest(request, authority):
	uri = 'http://' + authority + request.target
	return requests.request(request.method, uri, headers = request.headers, data = request.body)

# Evaluate the response
def evaluateResponse(response, expected, custom_evaluator):
	x = evaluateStatusCode(response.status_code, expected.status_code)
	if x is not None:
		return x
	x = evaluateHeaders(response.headers, expected.headers)
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

def evaluateBody(content, exp_content, should_cmp = True):
	if not should_cmp:
		return None
	if content != exp_content:
		return createMessage('Body', 'Len: ' + str(len(content)), 'Len: ' + str(len(exp_content)))
	return None
