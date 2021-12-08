import ParseTestCase
from testcase_generation.cgi import tester42
from testcase_generation import example
from testcase_generation import example2
from testcase_generation.LocationBlock import autoindex
from testcase_generation.LocationBlock import default_file
from testcase_generation.LocationBlock import notallowed
from testcase_generation.LocationBlock import resolution
from testcase_generation.Method import delete
from testcase_generation.Method import get
from testcase_generation.Method import post
from testcase_generation.Redirection import redirection

def generate():
	testcases = ParseTestCase.testCaseFromFiles()
	testcases.append(tester42.testCase42Get())
	testcases.append(tester42.testCase42Post())
	testcases.append(example.testCaseFailingTestCase())
	testcases.append(example.testCaseExampleCustomEvaluation())
	testcases.append(example.testCaseSecondTestCase())
	testcases.append(example.testCaseSimpleTestCase())
	testcases.append(example2.testCaseSimpleTestCase())
	testcases.append(autoindex.testCaseAutoindexOnGet())
	testcases.append(autoindex.testCaseAutoindexOnPost())
	testcases.append(autoindex.testCaseAutoindexOnDelete())
	testcases.append(autoindex.testCaseAutoindexOffGet())
	testcases.append(autoindex.testCaseAutoindexOffPost())
	testcases.append(autoindex.testCaseAutoindexOffDelete())
	testcases.append(autoindex.testCaseAutoindexIndexGet())
	testcases.append(autoindex.testCaseAutoindexIndexPost())
	testcases.append(autoindex.testCaseAutoindexIndexDelete())
	testcases.append(default_file.testCaseDefaultFileGetYes())
	testcases.append(default_file.testCaseDefaultFilePost())
	testcases.append(default_file.testCaseDefaultFileDelete())
	testcases.append(default_file.testCaseDefaultFileGetNo())
	testcases.append(default_file.testCaseDefaultFileSearch())
	testcases.append(notallowed.testCaseGetNotAllowed())
	testcases.append(notallowed.testCasePostNotAllowed())
	testcases.append(notallowed.testCaseDeleteNotAllowed())
	testcases.append(resolution.testCaseMatchSlash())
	testcases.append(resolution.testCaseBlockOne())
	testcases.append(resolution.testCaseBlockTwo())
	testcases.append(resolution.testCaseNonExistantBlock())
	testcases.append(delete.testCaseDelete())
	testcases.append(delete.testCaseDeleteDefaultFile())
	testcases.append(delete.testCaseDeleteNotFound())
	testcases.append(delete.testCaseDeleteForbidden())
	testcases.append(get.testCaseGet())
	testcases.append(get.testCaseGetNotFound())
	testcases.append(post.testCasePostUploadDirNew())
	testcases.append(post.testCasePostUploadDirOld())
	testcases.append(post.testCasePostRootDirNew())
	testcases.append(post.testCasePostRootDirOld())
	testcases.append(post.testCasePostForbiddenFile())
	testcases.append(post.testCasePostForbiddenDir())
	testcases.append(redirection.testCaseRedirect())
	return testcases
