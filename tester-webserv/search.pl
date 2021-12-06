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

my $PREFIX = 'testCase';

# returns a list like `testCaseFunctionName`
# from line `def testCaseFunctionName(...):`
sub getFunctionNames {
	my @function_names;
	my $regex_string = '/^def\s*(.*)\(.*\)/';

	for (@_) {
		open(my $FH, '<', $_) or die "$!\n";
		while (<$FH>) {
			# print "$_";
			if ($_ =~ /^def\s*($PREFIX.*)\(.*\)/) {
				push(@function_names, $1);
			}
		}
		close($FH) or die "$!\n";
	}

	return @function_names;
}

my @function_names = getFunctionNames(@ARGV);


# my $test_function_prefix = "testCase";
print "import ParseTestCase\n";

# USE the FILE BASENAME for `BASEFILENAME`
# # from TestCaseGeneration import BASEFILENAME

print "def generate():\n";
print "\ttestcases = ParseTestCase.testCaseFromFiles()\n";
for (@function_names) {
	print "\ttestcases.append($_())\n";
}


print "\treturn testcases\n";
