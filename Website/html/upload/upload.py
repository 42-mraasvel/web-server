import os
import sys
from requests_toolbelt.multipart import decoder

def printHeader(header):
	for field in header:
		print(field, ": ", header[field], sep = "", end = "\r\n")
	print(end = "\r\n")

def readBody():
	content = sys.stdin.read()
	return content

def postRequestImage(multipart_string):
	content_type = os.getenv("CONTENT_TYPE")

	x = decoder.MultipartDecoder(multipart_string, content_type)

	print(content_type, file = sys.stderr)

	if content_type == "text/plain":
		file = open('./test.txt', 'w')
	else:
		file = open('./test.bin', 'wb')
	for part in decoder.MultipartDecoder(multipart_string, content_type).parts:
		print(type(part.content), file = sys.stderr)
		file.write(part.content)
		pass
	file.close()

if __name__ == '__main__':
	if os.environ['REQUEST_METHOD'] != 'POST':
		exit(1)

	content = readBody()

	header = dict()
	header["status"] = 201
	printHeader(header)
	print(content)
