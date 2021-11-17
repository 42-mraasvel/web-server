import sys
import time

BYTE = 1
KILOBYTE = 1024 * BYTE
MEGABYTE = 1024 * KILOBYTE
GIGABYTE = 1024 * MEGABYTE

def printEnv():
	# import os for this
	print(os.environ)
	print(sys.argv)

def printHeader(header):
	for key in header:
		print(key, ": ", header[key], sep = '', end = "\r\n")
	
	print(end = "\r\n")

content = '''<html>
	<head>
		<title>Hello World - First CGI Program</title>
	</head>
	<body>
		<h2>Hello World! This is my first CGI program</h2>
	</body>
</html>
'''
content_type = 'text/html'

if __name__ == '__main__':
	header = dict()

	# Stress testing...

	# content += KILOBYTE * "A"
	# content += MEGABYTE * "A"
	# content += GIGABYTE * "A"

	header['Content-Type'] = content_type
	header['Content-Length'] = str(len(content))

	printHeader(header)
	print(content, end = "")



	print("Ending CGI", file = sys.stderr)
