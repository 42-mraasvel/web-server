def printHeader(header):
	if not header:
		return
	print(' -- HEADER FIELDS --')
	for key in header:
		print(key, ": ", header[key], sep = "")

def printBody(body):
	if not body:
		return
	print(' -- MESSAGE BODY --')
	if len(body) < 10000:
		print(body)
	else:
		print('Body Size:', len(body))

class Request:
	def __init__(self):
		self.method = str()
		self.target = str()
		self.headers = dict()
		self.body = bytes()

	def print(self):
		print(' -- Request -- ')
		print('method:', self.method)
		print('request_target:', self.target)
		printHeader(self.headers)
		printBody(self.body)

class Response:
	def __init__(self):
		self.status_code = int(0)
		self.headers = dict()
		self.expect_body = False
		self.body = bytes()

	def print(self):
		print(' -- Response --')
		print('Status Code:', self.status_code)
		print('Expect Body:', self.expect_body)
		printHeader(self.headers)
		printBody(self.body)

class TestCase:
	def __init__(self, request = None, response = None, tag = None):
		self.tag = tag
		self.request = request
		self.response = response
		if tag is None:
			self.tag = str()
		if request is None:
			self.request = Request()
		if response is None:
			self.response = Response()

	def print(self):
		self.request.print()
		self.response.print()
