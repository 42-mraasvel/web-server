#include "Utility/Output.hpp"
#include "TestCases/TestCase.hpp"
#include <unordered_set>
#include <string>

TestCaseVector generateTestCases();

static TagSet parseTags(int argc, char *argv[]) {
	// Skip program name
	argc--; argv++;
	TagSet tags;
	for (int i = 0; i < argc; ++i) {
		tags.insert(std::string(argv[i]));
	}
	return tags;
}

int main(int argc, char *argv[]) {
	Output::clearLog();
	const TagSet tags = parseTags(argc, argv);
	// executeTestCasesParallel(generateTestCases(), tags);
	executeTestCases(generateTestCases(), tags);
	return 0;
}
