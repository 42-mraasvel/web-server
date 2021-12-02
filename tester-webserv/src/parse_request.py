class Request:
	status_line = dict()
	header_fields = dict()
	message_body = str()

	def print(self):
		print(self.status_line['method'], self.status_line['target'], self.status_line['version'], end = "\r\n")
		for field in self.header_fields:
			print(field, ': ', self.header_fields[field], sep = "", end = "\r\n")
		print(end = "\r\n")
		print(self.message_body)

def _ParseStatusLine(status_str):
	status_list = status_str.split(' ')
	assert len(status_list) == 3

	status_line = dict()
	status_line['method'] = status_list[0]
	status_line['target'] = status_list[1]
	status_line['version'] = status_list[2]
	return status_line

def _ParseHeaderFields(header_str):

	fields = dict()
	lines = header_str.splitlines()
	for line in lines:
		name, value = line.split(':')
		fields[name.strip()] = value.strip()
	return fields

def _ParseContent(content_str):
	return content_str

def RequestFromFile(filename):
	request = Request()

	with open(filename, 'r') as f:
		content = f.read()

	header_start = content.find('\n')
	header_end = content.find('\n\n')

	request.status_line = _ParseStatusLine(content[:header_start])
	request.header_fields = _ParseHeaderFields(content[header_start + 1:header_end])
	request.message_body = _ParseContent(content[header_end + 2:])
	return request
