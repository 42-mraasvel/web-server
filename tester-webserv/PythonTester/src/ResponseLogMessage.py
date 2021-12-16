from TestCase import getHeaderList
from TestCase import getBodyList
import requests

def generateResponseLogMessage(response):
	lines = []

	lines.append('[INFO] Received Response:')
	lines.append('\tStatus Code: ' + str(response.status_code))
	lines.append('\tReason Phrase: \'' + response.reason + '\'')
	lines += getHeaderList(response.headers)
	lines += getBodyList(response.content)
	lines.append('')

	result = str()
	for line in lines:
		result += line + "\n"
	return result
