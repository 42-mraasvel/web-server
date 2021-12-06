import sys
import re

PREFIX = 'testCase'
SUBDIR = 'TestCaseGeneration'


#
# from <SUBDIR.subdir.subdir> import <file>
#

def findFunctions(args):
	# map[filename].append(function_name)
	map = dict()

	for arg in args:
		with open(arg, 'r') as f:
			# TODO: add ':$' at end
			regex_search = '^def\s*(' + PREFIX + '.*)\(.*\):$'
			for line in f.readlines():
				x = re.search(regex_search, line)
				if x:
					if arg in map:
						map[arg].append(x.group(1))
					else:
						map[arg] = [x.group(1)]
	return map

def printImportMessage(key):
	pass

def printFunctionCalls(key, functions):
	pass

def generateCode(map):
	print("import ParseTestCase")
	for key in map:
		printImportMessage(key)
	print("def generate():")
	print("\ttestcases = ParseTestCase.testCaseFromFiles()")
	for key in map:
		printFunctionCalls(key, map[key])
	print("\treturn testcases")

map = findFunctions(sys.argv[1:])
generateCode(map)
