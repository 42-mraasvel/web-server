import sys
import ExecuteTest
import Error
import Logger
import GenerateTestCases

USAGE_ERR_STR = "Arguments: [Hostname/IP:Port] [TestCase TAGS]"

def startup():
	Logger.clearLog()

if __name__ == '__main__':
	if len(sys.argv) == 1:
		Error.exitError(USAGE_ERR_STR)
	startup()
	authority = sys.argv[1]
	tags = sys.argv[2:]

	testcases = GenerateTestCases.generate()
	ExecuteTest.execute(testcases, authority, tags)
