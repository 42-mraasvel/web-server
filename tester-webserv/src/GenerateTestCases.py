FILENAME: src.TestCaseGeneration.test.py
FILENAME: src.TestCaseGeneration.subdir.example.py
FILENAME: src.TestCaseGeneration.subdir.example.py
FILENAME: src.TestCaseGeneration.subdir.example.py
FILENAME: src.TestCaseGeneration.subdir.example.py
FILENAME: src.TestCaseGeneration.subdir.example.py
FILENAME: src.TestCaseGeneration.subdir.subdir.test2.py
import ParseTestCase
from  import src.TestCaseGeneration.test
from  import src.TestCaseGeneration.subdir.example
from  import src.TestCaseGeneration.subdir.subdir.test2
def generate():
	testcases = ParseTestCase.testCaseFromFiles()
	testcases.append(src.TestCaseGeneration.test.py())
	testcases.append(src.TestCaseGeneration.subdir.example.py())
	testcases.append(src.TestCaseGeneration.subdir.example.pytestCaseFailingEvaluation())
	testcases.append(src.TestCaseGeneration.subdir.example.pytestCaseExampleCustomEvaluation())
	testcases.append(src.TestCaseGeneration.subdir.example.pytestCaseSecondTestCase())
	testcases.append(src.TestCaseGeneration.subdir.example.pytestCaseSimpleTestCase())
	testcases.append(src.TestCaseGeneration.subdir.subdir.test2.py())
	return testcases
