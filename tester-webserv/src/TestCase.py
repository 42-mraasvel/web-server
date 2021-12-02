class Request:
	method = str()
	request_target = str()
	authority = str()
	header = dict()
	body = str()

	def print(self):
		print(' -- Request -- ')
		print('method:',self.method)
		print('request_target:',self.request_target)
		print('authority:',self.authority)
		print('  -- header fields --')
		for field in self.header:
			print(field)
		if len(self.body) < 10000:
			print(self.body)
		else:
			print('body too big to print')


class Response:
	status_code = int()
	header = dict()
	body = str()

	def print(self):
		print(' -- Response --')
		print(self.status_code)
		for field in self.header:
			print(field)
		if len(self.body) < 10000:
			print(self.body)
		else:
			print('body too big to print')

class TestCase:
	request = Request()
	response = Response()
	tag = str()

	def print(self):
		self.request.print()
		self.response.print()
