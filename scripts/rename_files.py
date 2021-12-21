import glob
import os
import sys
import re

PATH = sys.argv[1]
result = [y for x in os.walk(PATH) for y in glob.glob(os.path.join(x[0], '*.conf'))]

for file in result:
	regex = "(.*)ignore_(.*\.conf)"
	result = re.search(regex, file)
	if result:
		newpath = result.groups(1)[0] + result.groups(1)[1]
		print(file, "->", newpath)
		# os.rename(file, newpath)
		# print(result.groups(1)[0] + result.groups(1)[1])

# to rename: os.rename(oldpath, newpath)
