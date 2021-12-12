import ParseTestCase
from testcase_generation.Method import get
from testcase_generation.Method import delete
from testcase_generation.Method import post
from testcase_generation.cgi import tester42
from testcase_generation.LocationBlock import autoindex
from testcase_generation.LocationBlock import resolution
from testcase_generation.LocationBlock import notallowed
from testcase_generation.LocationBlock import redirection
from testcase_generation.LocationBlock import default_file
from testcase_generation.LocationBlock import root
from testcase_generation.Response import response_header
from testcase_generation.Response import media_type
from testcase_generation.Response import status_code
from testcase_generation.ServerBlock import server_name
from testcase_generation.ServerBlock import max_body_size
from testcase_generation.ServerBlock import default_errorpage

def generate():
	testcases = ParseTestCase.testCaseFromFiles()
	testcases.append(get.testCaseGet())
	testcases.append(get.testCaseGetNotFound())
	testcases.append(delete.testCaseDelete())
	testcases.append(delete.testCaseDeleteDefaultFile())
	testcases.append(delete.testCaseDeleteNotFound())
	testcases.append(delete.testCaseDeleteForbiddenFile())
	testcases.append(delete.testCaseDeleteForbiddenDir())
	testcases.append(post.testCasePostUploadDirNew())
	testcases.append(post.testCasePostUploadDirOld())
	testcases.append(post.testCasePostRootDirNew())
	testcases.append(post.testCasePostRootDirOld())
	testcases.append(post.testCasePostForbiddenFile())
	testcases.append(post.testCasePostForbiddenDir1())
	testcases.append(post.testCasePostForbiddenDir2())
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
	testcases.append(resolution.testCaseResolutionMatchSlash())
	testcases.append(resolution.testCaseResolutionBlockOne())
	testcases.append(resolution.testCaseResolutionBlockTwo())
	testcases.append(resolution.testCaseResolutionNonExistantBlock())
	testcases.append(notallowed.testCaseGetNotAllowed())
	testcases.append(notallowed.testCasePostNotAllowed())
	testcases.append(notallowed.testCaseDeleteNotAllowed())
	testcases.append(redirection.testCaseRedirectGet())
	testcases.append(redirection.testCaseRedirectPost())
	testcases.append(redirection.testCaseRedirectDelete())
	testcases.append(default_file.testCaseDefaultFileGetYes())
	testcases.append(default_file.testCaseDefaultFilePost())
	testcases.append(default_file.testCaseDefaultFileDelete())
	testcases.append(default_file.testCaseDefaultFileGetNo())
	testcases.append(default_file.testCaseDefaultFileSearch())
	testcases.append(root.testCaseRootEmpty())
	testcases.append(root.testCaseRootErrorFile())
	testcases.append(root.testCaseRootError1())
	testcases.append(root.testCaseRootError2())
	testcases.append(root.testCaseRootError3())
	testcases.append(root.testCaseRootError4())
	testcases.append(root.testCaseRootError5())
	testcases.append(response_header.testCaseConnectionClose())
	testcases.append(media_type.testCaseMediaType_html())
	testcases.append(media_type.testCaseMediaType_txt())
	testcases.append(media_type.testCaseMediaType_png())
	testcases.append(media_type.testCaseMediaType_jpeg())
	testcases.append(media_type.testCaseMediaType_sh())
	testcases.append(status_code.testCaseStatusCode417())
	testcases.append(status_code.testCaseStatusCode501Method())
	testcases.append(status_code.testCaseStatusCode501Encoding())
	testcases.append(server_name.testCaseServerName8080Default())
	testcases.append(server_name.testCaseServerName8081Default())
	testcases.append(server_name.testCaseServerName8081First())
	testcases.append(server_name.testCaseServerName8081Second())
	testcases.append(server_name.testCaseServerNameWildcardExactMatch())
	testcases.append(server_name.testCaseServerNameWildcardFront())
	testcases.append(server_name.testCaseServerNameWildcardFrontPriority())
	testcases.append(server_name.testCaseServerNameWildcardBack())
	testcases.append(max_body_size.testCaseMaxBodySizeGet())
	testcases.append(max_body_size.testCaseMaxBodySizePost())
	testcases.append(max_body_size.testCaseMaxBodySizeDelete())
	testcases.append(default_errorpage.testCaseDefaultErrorPageRedirect1())
	testcases.append(default_errorpage.testCaseDefaultErrorPageRedirect2())
	testcases.append(default_errorpage.testCaseDefaultErrorPageRedirectNotFound())
	testcases.append(default_errorpage.testCaseDefaultErrorPageNotRedirect())
	return testcases
