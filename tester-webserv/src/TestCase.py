def printHeader(header):
	if not header:
		return
	print(' -- header fields --')
	for key in header:
		print(key, ": ", header[key], sep = "")

def printBody(body):
	if not body:
		return
	if len(body) < 10000:
		print(body)
	else:
		print('body too big to print')

class Request:
	method = str()
	target = str()
	authority = str()
	headers = dict()
	body = str()

	def print(self):
		print(' -- Request -- ')
		print('method:', self.method)
		print('request_target:', self.target)
		print('authority:', self.authority)
		printHeader(self.headers)
		printBody(self.body)

class Response:
	status_code = int()
	headers = dict()
	expect_body = bool()
	body = bytes()

	def __init__(self):
		self.status_code = 0
		self.expect_body = False

	def print(self):
		print(' -- Response --')
		print('Status Code:', self.status_code)
		print('Expect Body:', self.expect_body)
		printHeader(self.headers)
		printBody(self.body)

class TestCase:
	request = Request()
	response = Response()
	tag = str()

	def print(self):
		self.request.print()
		self.response.print()
