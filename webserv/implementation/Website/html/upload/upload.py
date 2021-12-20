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

def postContent(filename, content):
	status_code = 201
	if os.path.exists(filename):
		status_code = 204
	with open(filename, "wb") as f:
		f.write(content)
	return status_code

def postPart(part):
	global response_body
	disposition = part.headers[b'Content-Disposition'].decode('ascii')
	regex = re.search('.*filename="(.*)"', disposition)
	filename = regex.groups(1)[0]
	headers['Location'] = '/' + UPLOAD_STORE + filename
	response_body += "Uploaded: " + filename + "\r\n"
	return postContent("./" + UPLOAD_STORE + filename, part.content)

def multipartPost(body, content_type):
	x = decoder.MultipartDecoder(body, content_type)
	status_code = 204
	for part in x.parts:
		status_code = postPart(part)
	return status_code

def postRequestImage(content):
	global response_body
	content_type = os.getenv("CONTENT_TYPE")
	if "multipart/form-data" in content_type:
		return multipartPost(content, content_type)
	else:
		response_body = "raw content posted"
		return postContent("./" + UPLOAD_STORE + "raw.txt", content)

def cgiError(status_code, msg):
	headers["status"] = status_code
	printHeader(headers)
	print(msg)
	exit(1)

if __name__ == '__main__':
	if os.environ['REQUEST_METHOD'] != 'POST':
		cgiError(500, "CGI: method: POST required")

	content = readBody()

	createUploadStore()
	headers["Status"] = postRequestImage(content)

	printHeader(headers)
	if headers["Status"] == 201:
		print(response_body)
