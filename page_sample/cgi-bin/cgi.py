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

	header['Content-Type'] = content_type
	header['Content-Length'] = str(len(content))

	printHeader(header)
	print(content, end = "")
