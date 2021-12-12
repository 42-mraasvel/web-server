#pragma once

#include "Client/Client.hpp"
#include "DataStructures/Request.hpp"
#include "ResponseValidator/ResponseValidator.hpp"
#include <vector>

struct TestCase {
	public:
		typedef Client::RequestPair RequestPair;
		typedef Client::RequestQueue RequestQueue;
		typedef Client::Settings Settings;

	public:
		RequestQueue requests;
		Settings settings;
		Address server;
};

typedef std::vector<TestCase> TestCaseVector;

void executeTestCases(TestCaseVector testcases);
