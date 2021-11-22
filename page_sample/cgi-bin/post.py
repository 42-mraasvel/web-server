from sys import stdin
import os

def printHeader(header):
	for key in header:
		print(key, ": ", header[key], sep = '', end = "\r\n")
	
	print(end = "\r\n")

def printResponse(content, header):
	printHeader(header)
	print(content, end = "")

def doResponse(content):
	header = dict()

	header['Content-Type'] = 'text/plain'
	header['Content-Length'] = len(content)
	header['Status'] = 200

	printResponse(content, header)

def readRequest():
	content = str()
	for line in stdin:
		content += line
	return content

# 1. Read until EOF
# 2. Print basic response

if __name__ == '__main__':

	content = readRequest()
	content += os.getcwd() + "\r\n"
	doResponse(content)
