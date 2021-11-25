import os
import sys

def printHeader(header):
	for key in header:
		print(key, ": ", header[key], sep = '', end = "\r\n")
	
	print(end = "\r\n")

def printEnv():
	for x in os.environ:
		print(x, '=', os.environ[x], sep = "")
	for x in sys.argv:
		print(x)

if __name__ == '__main__':
	header = dict()
	header['Content-Type'] = 'text/plain'
	printHeader(header)
	printEnv()
