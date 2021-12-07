import Error
import TestCase
import os

DIRECTORY = './Requests'

def exitParseError(message):
	Error.exitError('ParseTestCase: ' + message)

def validExtension(ext):
	return ext == '.txt' or ext == '.response'

#
# Returns list of testcases in the Directory `Requests`
#
def testCaseFromFiles():
	testcases = []
	files = [os.path.join(dp, f) for dp, dn, filenames in os.walk(DIRECTORY) for f in filenames if validExtension(os.path.splitext(f)[1])]
	for file in files:
		if file.endswith('.txt'):
			if file + '.response' not in files:
				exitParseError('{}: no matching response file: {}.response'.format(file, file))
			testcases.append(testCaseFromFile(file))
		elif not file.endswith('.txt.response'):
			exitParseError('bad filename: {} expected: [.txt or .txt.response] '.format(file))
	return testcases

def testCaseFromFile(filename):
	testcase = TestCase.TestCase(parseRequest(filename), parseResponse(filename + '.response'), filename, filename)
	return testcase

def parseRequest(filename):
	return parseFile(filename, parseRequestLine)

def parseResponse(filename):
	response = parseFile(filename, parseStatusLine)
	if len(response.body) > 0:
		response.expect_body = True
	return response

def parseFile(filename, parseline_function):
	with open(filename, 'r') as f:
		content = f.read()
	
	start_header = content.find('\n')
	end_header = content.find('\n\n')
	if start_header <= 0:
		exitParseError('first line not found (no newline or no content): {}'.format(filename))
	result = parseline_function(content[:start_header], filename)
	if end_header == -1:
		return result
	result.headers = parseHeader(content[start_header + 1:end_header], filename)
	result.body = parseBody(content[end_header + 2:], filename)
	return result

def parseRequestLine(requestline, filename):
	parts = requestline.split(' ')
	if len(parts) != 3:
		exitParseError('{}: invalid request line: {}'.format(filename, requestline))
	request = TestCase.Request()
	request.method = parts[0]
	request.target = parts[1]
	return request

def parseStatusLine(statusline, filename):
	parts = statusline.split(' ')
	if len(parts) != 3:
		exitParseError('{}: invalid status line: {}'.format(filename, requestline))
	response = TestCase.Response()
	response.status_code = int(parts[1])
	return response

def parseHeader(header_string, filename):
	headers = dict()
	if (len(header_string) == 0):
		return headers
	lines = header_string.split('\n')
	for line in lines:
		index = line.find(":")
		if index <= 0:
			exitParseError('{}: invalid header field: {}'.format(filename, line))
		headers[line[:index].strip()] = line[index + 1:].strip()
	return headers

def parseBody(content, filename):
	return content
