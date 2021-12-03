def getHeaderList(header):
	data = []
	if not header:
		return data
	data.append('  -- HEADER FIELDS --')
	for key in header:
		data.append(key + ": " + header[key])
	return data

def getBodyList(body):
	data = []
	data.append('  -- MESSAGE BODY --')
	if not body or len(body) > 10000 or type(body) == bytes:
		data.append('Body Size: ' + str(len(body)))
	else:
		data.append(body)
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
		data.append('-- Request -- ')
		data.append('method: ' + self.method)
		data.append('request_target: ' + self.target)
		data += getHeaderList(self.headers)
		data += getBodyList(self.body)
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
		data.append('-- Response --')
		data.append('Status Code: ' + str(self.status_code))
		data.append('Expect Body: ' + str(self.expect_body))
		data += getHeaderList(self.headers)
		data += getBodyList(self.body)
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
			self.tag = str()
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
		result = str()
		for line in lines:
			if line[0] == ' ' or line[0] == '-':
				result += line + "\n"
			else:
				result += "\t" + line + "\n"
		return result
