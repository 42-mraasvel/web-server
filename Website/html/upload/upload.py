import os
import sys
import re
from requests_toolbelt.multipart import decoder

UPLOAD_STORE = "upload/UploadedFiles/"

headers = dict()
response_body = ""

def createUploadStore():
	os.makedirs("./" + UPLOAD_STORE, exist_ok = True)

def printHeader(header):
	for field in header:
		print(field, ": ", header[field], sep = "", end = "\r\n")
	print(end = "\r\n")

def readBody():
	content = sys.stdin.buffer.read()
	return content

def postPart(part):
	global response_body
	disposition = part.headers[b'Content-Disposition'].decode('ascii')
	regex = re.search('.*filename="(.*)"', disposition)
	filename = regex.groups(1)[0]
	with open("./" + UPLOAD_STORE + filename, 'wb') as f:
		f.write(part.content)
	headers['Location'] = '/' + UPLOAD_STORE + filename
	response_body += "Uploaded: " + filename + "\r\n"

def multipartPost(body, content_type):
	x = decoder.MultipartDecoder(body, content_type)
	for part in x.parts:
		postPart(part)

def postRawData(content):
	with open ('./raw.txt', 'wb') as f:
		f.write(content)

def postRequestImage(content):
	content_type = os.getenv("CONTENT_TYPE")
	if "multipart/form-data" in content_type:
		multipartPost(content, content_type)
	else:
		postRawData(content)

def cgiError(status_code, msg):
	headers["status"] = status_code
	printHeader(headers)
	print(msg)
	exit(1)

if __name__ == '__main__':
	if os.environ['REQUEST_METHOD'] != 'POST':
		cgiError(500, "CGI: method: POST required")

	createUploadStore()
	content = readBody()
	headers["status"] = 201
	postRequestImage(content)
	printHeader(headers)
	if len(response_body) > 0:
		print(response_body)
	else:
		print("Nothing Posted")
