import sys
import ExecuteTest
import Error
import Logger
import GenerateTestCases

USAGE_ERR_STR = "Arguments: [Hostname/IP:Port]"

def startup():
	Logger.clearLog()

if __name__ == '__main__':
	if len(sys.argv) != 2:
		Error.exitError(USAGE_ERR_STR)
	startup()
	authority = sys.argv[1]

	testcases = GenerateTestCases.generate()
	ExecuteTest.execute(testcases, authority)
