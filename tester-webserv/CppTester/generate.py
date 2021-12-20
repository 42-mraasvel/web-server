import sys
import re

GENERATOR_START = \
"""
TestCaseVector generateTestCases() {
	TestCaseVector testcases;"""
GENERATOR_END = \
"""\treturn testcases;
}"""

def extractFunctionNames(lines, prefix):
	names = list()
	regex = "^TestCase\s*(" + prefix + ".*)\(\).*$"
	for line in lines:
		x = re.search(regex, line)
		if x:
			names.append(x.group(1))
	return names

def findFunctions(files, prefix):
	# List of function names
	functions = list()
	for file in files:
		with open(file, 'r') as f:
			functions += extractFunctionNames(f.readlines(), prefix)
	return functions

def printPrototype(function):
	print("TestCase ", function, "();", sep = "")

def printPushBack(function):
	print("\ttestcases.push_back(", function, "());", sep = "")

def generateCode(functions):
	for function in functions:
		printPrototype(function)
	print(GENERATOR_START)
	for function in functions:
		printPushBack(function)
	print(GENERATOR_END)

# Arguments: [PREFIX] [FILES]
if __name__ == "__main__":
	prefix = sys.argv[1]
	print("#include \"TestCase.hpp\"")
	print()
	functions = findFunctions(sys.argv[2:], prefix)
	generateCode(functions)
