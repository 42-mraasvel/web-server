import sys
import re
import os

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
					key = os.path.splitext(arg)[0]
					if key in map:
						map[key].append(x.group(1))
					else:
						map[key] = [x.group(1)]
	return map

# from SUBDIR.subdir.subdir import basename(file) (no extension)
def printImportMessage(key):
	dir = os.path.dirname(key)
	filename = os.path.basename(key)
	print("from", dir[dir.find(TESTDIR):].replace('/', '.'), "import", filename)

def printFunctionCalls(key, functions):
	import_name = os.path.basename(key)
	for function in functions:
		print("\ttestcases.append(", import_name, ".", function, "())", sep = "")
	pass

def generateCode(map):
	print("import ParseTestCase")
	for key in map:
		printImportMessage(key)
	print()
	print("def generate():")
	print("\ttestcases = ParseTestCase.testCaseFromFiles()")
	for key in map:
		printFunctionCalls(key, map[key])
	print("\treturn testcases")

TESTDIR = sys.argv[1]
PREFIX = sys.argv[2]
map = findFunctions(sys.argv[3:])
generateCode(map)
