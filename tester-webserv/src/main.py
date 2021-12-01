import sys
import socket
import requests

USAGE_ERR_STR = "Arguments: [Hostname/IP:Port]"
RED_BOLD = "\033[1;31m"
RESET_COLOR = "\033[0m"

def exitError(message):
	assert type(message) == str
	print(RED_BOLD + "ERROR:" + RESET_COLOR, message, file = sys.stderr)
	exit(1)

class HttpTester:
	_uri = str()

	def __init__(self, host):
		self._uri = self.createUri(host)

	def createUri(self, host):
		return "http://" + host

	def print(self):
		print("-- HttpTester --")
		print("  ", self._uri)

	def get(self):
		print("Getting:", self._uri)
		return requests.get(self._uri)

if __name__ == '__main__':
	if len(sys.argv) != 2:
		exitError(USAGE_ERR_STR)

	http_tester = HttpTester(sys.argv[1])

	http_tester.print()

	response = http_tester.get()
	print(response)
	response.close()
