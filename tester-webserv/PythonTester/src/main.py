import sys
import ExecuteTest
import Error
import Logger
import GenerateTestCases
import Constants

USAGE_ERR_STR = "Arguments: [TestCase TAGS]"

def startup():
	Logger.clearLog()
	sys.path.insert(0, './TestCaseGeneration')

if __name__ == '__main__':
	if len(sys.argv) == 0:
		Error.exitError(USAGE_ERR_STR)
	startup()
	tags = sys.argv[2:]
	Constants.SERVER_ROOT = sys.argv[1]

	testcases = GenerateTestCases.generate()
	exit_status = ExecuteTest.execute(testcases, tags)
	exit(exit_status)
