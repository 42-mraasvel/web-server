use strict;
use warnings;
# Prefix for a function:
# Generates python file to dynamically add testcases to the generator
# 
# 

# 1. All files given as arguments through Makefile
# 2. Each file is scanned for def PREFIXfunctionName(): (regex match)
# 3. Add each function string to a list
# 4. Generate the appropriate imports and function calls


# def generate():
# 	testcases = ParseTestCase.testCaseFromFiles()
# 	testcases.append(testCaseSampleTestCase())
# 	return testcases

# returns a list like `testCaseFunctionName`
# from line `def testCaseFunctionName(...):`
sub getFunctionNames {
	my @function_names;


	for (@_) {
		open(my $FH, '<', $_) or die "$!\n";
		push(@function_names, $_);

		close($FH) or die "$!\n";
	}

	return @function_names;
}

my @function_names = getFunctionNames(@ARGV);

for (@function_names) {
	print "$_\n";
}

# my $test_function_prefix = "testCase";
# print "import ParseTestCase\n";

# # from TestCaseGeneration import test

# print "def generate():\n";
# print "\ttestcases = ParseTestCase.testCaseFromFiles()\n";

# print "\treturn testcases\n";
