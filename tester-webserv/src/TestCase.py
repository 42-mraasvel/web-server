import chardet

def getHeaderList(header):
	data = []
	if not header:
		return data
	data.append("[INFO] [Header Fields]")
	for key in header:
		data.append('\t' + key + ": " + header[key])
	return data

def decodeBody(body):
	encoding = chardet.detect(body)['encoding']
	if encoding != 'ascii':
		return '[WARNING] unknown encoding: ' + encoding
	return body.decode(encoding)

def getBodyList(body):
	data = []
	data.append("[INFO] [Message Body]")
	if not body or len(body) > 10000:
		data.append('\tBody Size: ' + str(len(body)))
	elif type(body) == bytes:
		data.append(decodeBody(body))
	else:
		data.append(str(body))
	return data

class Request:
	def __init__(self):
		self.method = str()
		self.target = str()
		self.headers = dict()
		self.body = bytes()
		self.authority = 'localhost:8080'

	def print(self):
		for line in self.getLogList():
			print(line)
	
	def getLogList(self):
		data = []
		data.append('[INFO] Request:')
		data.append('\tmethod: ' + self.method)
		data.append('\trequest_target: ' + self.target)
		data += getHeaderList(self.headers)
		data += getBodyList(self.body)
		data.append('')
		return data

class Response:
	def __init__(self):
		self.status_code = int(0)
		self.headers = dict()
		self.expect_body = False
		self.body = bytes()
		self.headers['connection'] = 'keep-alive'

	def print(self):
		for line in self.getLogList():
			print(line)

	def getLogList(self):
		data = []
		data.append('[INFO] Expected Response:')
		data.append('\tStatus Code: ' + str(self.status_code))
		data.append('\tExpect Body: ' + str(self.expect_body))
		data += getHeaderList(self.headers)
		data += getBodyList(self.body)
		data.append('')
		return data

def defaultResponseEvaluator(response):
	if len(response.headers['Date']) != 29:
		return "HeaderField 'Date: {}' has incorrect length (Expected: 29)".format(response.headers['Date'])
	if (response.status_code >= 300 and response.status_code < 400) or response.status_code == 204 or response.status_code == 304:
		if response.content:
			return "Unexpected message body"
	if response.status_code >= 300 and response.status_code < 400:
		if 'Location' not in response.headers:
			return "Expected Headerfield 'Location' not found"
		if 'Retry-After' not in response.headers:
			return "Expected Headerfield 'Retry-After' not found"
	if 'Transfer-Encoding' in response.headers and 'Content-Length' in response.headers:
		return "Transfer-Encoding and Content-Length present togehter"
	return None

class TestCase:
	def __init__(self, request = None, response = None, tag = None, name = None, evaluator = None):
		self.tag = tag
		self.name = name
		self.request = request
		self.response = response
		self.evaluator = evaluator
		if tag is None:
			self.tag = ""
		if name is None:
			self.name = ""
		if request is None:
			self.request = Request()
		if response is None:
			self.response = Response()
		if evaluator is None:
			self.evaluator = defaultResponseEvaluator

	def print(self):
		self.request.print()
		self.response.print()
	
	def getLogString(self):
		lines = self.request.getLogList() + self.response.getLogList()
		result = '[INFO] TestCase Tag/Name: \'{}-{}\'\n'.format(self.tag, self.name)
		for line in lines:
			result += line + "\n"
		return result
