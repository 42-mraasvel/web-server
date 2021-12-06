use strict;
use warnings;
use File::Basename;

my $PREFIX = 'testCase';
my $SUBDIR = 'TestCaseGeneration';

# Import: TestCaseGeneration.subdir1.subdir2 import $BASEFILENAME
sub getDirectory {
	my $path = $_[0];
	my $filename = $_[1];

	$path =~ /.*\/($SUBDIR.*)\/$filename/;
	my $dir = $1;
	$dir =~ s/\//\./g;
	return $dir;
}

sub printImports {
	print "import ParseTestCase\n";
	for (@_) {
		my $path = $_;
		my $filename = basename($_);
		my $dirname = getDirectory($path, $filename);
		print "from $dirname import $filename\n";
	}
}

sub printFunction {
	print "def generate():\n";
	print "\ttestcases = ParseTestCase.testCaseFromFiles()\n";
	for (@_) {
		print "\ttestcases.append($_())\n";
	}
	print "\treturn testcases\n";
}

# returns a list like `testCaseFunctionName`
# from line `def testCaseFunctionName(...):`
sub generateCode {
	my @function_names;
	my @filenames;
	my $regex_string = '/^def\s*(.*)\(.*\)/';

	for (@_) {
		my $filename = $_;
		open(my $FH, '<', $filename) or die "$!\n";
		while (<$FH>) {
			# print "$_";
			if ($_ =~ /^def\s*($PREFIX.*)\(.*\)/) {
				$filename =~ s/\.[^.]+$//;
				print "FILENAME: $filename\n";
				push(@function_names, basename($filename).".$1");
				# add filename if it's not yet present
				if (!(grep( /^$filename$/, @filenames))) {
					push(@filenames, $filename);
				}
			}
		}
		close($FH) or die "$!\n";
	}

	printImports(@filenames);
	printFunction(@function_names);

}

exit if @ARGV == 0;
generateCode(@ARGV);
