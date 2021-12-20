import os
import sys

def printHeader(header):
	for field in header:
		print(field, ": ", header[field], sep = "", end = "\r\n")
	print(end = "\r\n")

def readBody():
	content = sys.stdin.read()
	return content

if __name__ == '__main__':
	if os.environ['REQUEST_METHOD'] != 'POST':
		exit(1)

	content = readBody()

	header = dict()
	header["status"] = 201
	printHeader(header)
	print(content)
