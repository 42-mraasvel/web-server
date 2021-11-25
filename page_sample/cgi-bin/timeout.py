import time
import sys

def printHeader(header):
	for key in header:
		print(key, ": ", header[key], sep = '', end = "\r\n")
	
	print(end = "\r\n")


header = dict()
header["1"] = "1"
header["2"] = "2"

content = "1234"

time.sleep(5)
header["Content-Length"] = len(content)
printHeader(header)

print(end = "", flush = True)

while True:
	pass

print(content, end = "")
