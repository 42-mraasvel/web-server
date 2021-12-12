#include "Utility/Output.hpp"
#include "TestCases/TestCase.hpp"

TestCaseVector generateTestCases();

int main() {
	Output::clearLog();
	executeTestCases(generateTestCases());
	return 0;
}
