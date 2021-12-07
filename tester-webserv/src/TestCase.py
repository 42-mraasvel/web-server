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
	return None

class TestCase:
	def __init__(self, request = None, response = None, tag = None, evaluator = None):
		self.tag = tag
		self.request = request
		self.response = response
		self.evaluator = evaluator
		if tag is None:
			self.tag = "DEFAULT_TAG"
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
		result = '[INFO] TestCase Tag/Name: \'{}\'\n'.format(self.tag)
		for line in lines:
			result += line + "\n"
		return result
