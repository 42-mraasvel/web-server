from sys import stdin
import os
# from requests_toolbelt.multipart import decoder
import sys

def printHeader(header):
	for key in header:
		print(key, ": ", header[key], sep = '', end = "\r\n")
	
	print(end = "\r\n")

def printResponse(content, header):
	printHeader(header)
	print(content, end = "")

def doResponse():
	header = dict()

	header['Status'] = 200
	printResponse("", header)

def readRequest():
	data = sys.stdin.buffer.read()
	return data

def postRequestImage(multipart_string):
	content_type = os.getenv("CONTENT_TYPE")

	x = decoder.MultipartDecoder(multipart_string, content_type)

	print(content_type, file = sys.stderr)

	file = open('./img.png', 'wb')
	for part in decoder.MultipartDecoder(multipart_string, content_type).parts:
		print(type(part.content), file = sys.stderr)
		file.write(part.content)
		pass
	file.close()

def postRawData(content):
	with open ('./posting/post', 'wb') as f:
		f.write(content)

def executeRequest(content):
	postRawData(content)
	# postRequestImage(content)

# 1. Read until EOF
# 2. Print basic response

if __name__ == '__main__':

	content = readRequest()
	executeRequest(content)
	doResponse()
