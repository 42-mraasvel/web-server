import ParseTestCase
from testcase_generation.cgi import tester42
from testcase_generation.locationblock import autoindex
from testcase_generation.locationblock import default_file
from testcase_generation.locationblock import notallowed
from testcase_generation.locationblock import redirection
from testcase_generation.locationblock import resolution
from testcase_generation.locationblock import root
from testcase_generation.Method import delete
from testcase_generation.Method import get
from testcase_generation.Method import post
from testcase_generation.ServerBlock import default_errorpage
from testcase_generation.ServerBlock import max_body_size
from testcase_generation.ServerBlock import server_name

def generate():
	testcases = ParseTestCase.testCaseFromFiles()
	testcases.append(tester42.testCase42Get())
	testcases.append(tester42.testCase42Post())
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
	testcases.append(redirection.testCaseRedirectGet())
	testcases.append(redirection.testCaseRedirectPost())
	testcases.append(redirection.testCaseRedirectDelete())
	testcases.append(resolution.testCaseMatchSlash())
	testcases.append(resolution.testCaseBlockOne())
	testcases.append(resolution.testCaseBlockTwo())
	testcases.append(resolution.testCaseNonExistantBlock())
	testcases.append(root.testCaseRootEmpty())
	testcases.append(root.testCaseRootError1())
	testcases.append(root.testCaseRootError2())
	testcases.append(root.testCaseRootError3())
	testcases.append(root.testCaseRootError4())
	testcases.append(root.testCaseRootError5())
	testcases.append(delete.testCaseDelete())
	testcases.append(delete.testCaseDeleteDefaultFile())
	testcases.append(delete.testCaseDeleteNotFound())
	testcases.append(delete.testCaseDeleteForbiddenFile())
	testcases.append(delete.testCaseDeleteForbiddenDir())
	testcases.append(get.testCaseGet())
	testcases.append(get.testCaseGetNotFound())
	testcases.append(get.testCaseGetForbiddenFile())
	testcases.append(post.testCasePostUploadDirNew())
	testcases.append(post.testCasePostUploadDirOld())
	testcases.append(post.testCasePostRootDirNew())
	testcases.append(post.testCasePostRootDirOld())
	testcases.append(post.testCasePostForbiddenFile())
	testcases.append(post.testCasePostForbiddenDir1())
	testcases.append(post.testCasePostForbiddenDir2())
	testcases.append(default_errorpage.testCaseDefaultErrorPageRedirect1())
	testcases.append(default_errorpage.testCaseDefaultErrorPageRedirect2())
	testcases.append(default_errorpage.testCaseDefaultErrorPageRedirectNotFound())
	testcases.append(default_errorpage.testCaseDefaultErrorPageNotRedirect())
	testcases.append(max_body_size.testCaseMaxBodySizeGet())
	testcases.append(max_body_size.testCaseMaxBodySizePost())
	testcases.append(max_body_size.testCaseMaxBodySizeDelete())
	testcases.append(server_name.testCaseServerNameNotFound())
	return testcases
