#include "TestCase.hpp"
#include "Client/Client.hpp"
#include "Utility/utility.hpp"
#include "Utility/macros.hpp"
#include <sys/wait.h>
#include <unistd.h>
#include <sys/types.h>

static void executeTestCase(const TestCase& testcase) {
	Client::testRequests(testcase.requests, testcase.server, testcase.settings);
}

static void nameRequests(TestCase& testcase) {
	for (TestCase::RequestPair request_pair : testcase.requests) {
		request_pair.first->name = testcase.name;
		request_pair.first->tag = testcase.tag;
	}
}

static void waitPids(std::vector<pid_t> pids) {
	for (pid_t pid : pids) {
		int ret = waitpid(pid, NULL, 0);
		if (ret == -1) {
			syscallError(_FUNC_ERR("waitpid"));
		}
	}
}

static bool shouldExecuteTestcase(const TestCase& testcase, const TagSet tags) {
	return (tags.size() == 0 && !testcase.execute_only_if_tag)
		|| tags.count(testcase.tag) > 0;
}

void executeTestCasesParallel(TestCaseVector testcases, const TagSet tags) {
	std::vector<pid_t> pids;
	for (TestCase& testcase : testcases) {
		if (!shouldExecuteTestcase(testcase, tags)) {
			continue;
		}
		pid_t pid = fork();
		if (pid == -1) {
			syscallError(_FUNC_ERR("fork"));
			break;
		} else if (pid == 0) {
			nameRequests(testcase);
			executeTestCase(testcase);
			exit(0);
		} else {
			pids.push_back(pid);
		}
	}
	waitPids(pids);
}

void executeTestCases(TestCaseVector testcases, const TagSet tags) {
	for (TestCase& testcase : testcases) {
		if (!shouldExecuteTestcase(testcase, tags)) {
			continue;
		}
		nameRequests(testcase);
		executeTestCase(testcase);
	}
}
