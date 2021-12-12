#pragma once

#include "Client/Client.hpp"
#include "DataStructures/Request.hpp"
#include "ResponseValidator/ResponseValidator.hpp"
#include "TestCaseUtils/TestCaseUtil.hpp"
#include <vector>
#include <unordered_set>
#include <string>

namespace Constants {
extern std::string SERVER_ROOT;
}

struct TestCase {
	public:
		typedef Client::RequestPair RequestPair;
		typedef Client::RequestQueue RequestQueue;
		typedef Client::Settings Settings;

	public:
		std::string tag;
		std::string name;
	public:
		RequestQueue requests;
		Settings settings;
		Address server;
};

typedef std::vector<TestCase> TestCaseVector;
typedef std::unordered_set<std::string> TagSet;

void executeTestCases(TestCaseVector testcases, const TagSet tags = {});
