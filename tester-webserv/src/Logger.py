LOGFILENAME = "result.log"

def log(message):
	with open(LOGFILENAME, 'a') as f:
		f.write(message)

def clearLog():
	with open(LOGFILENAME, 'w') as f:
		pass
