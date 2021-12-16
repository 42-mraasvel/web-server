import sys
import Logger

RED_BOLD = "\033[1;31m"
RESET_COLOR = "\033[0m"

def putMsg(message):
	print(RED_BOLD + "ERROR" + RESET_COLOR + ':', message, file = sys.stderr)

def putFail(message):
	print(RED_BOLD + "FAIL" + RESET_COLOR + ':', message, file = sys.stderr)
	Logger.log('[ERROR] ' + message + "\n")

def exitError(message):
	putMsg(message)
	exit(1)
