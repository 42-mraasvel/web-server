# Settings for the testcase generation
TESTCASEDIR := testcase_generation
PREFIX := testCase

# Details, details
SDIR = src
FULLPATH := $(SDIR)/$(TESTCASEDIR)

TESTFILES := $(shell find $(FULLPATH) -name "*.py" -type f)
OUTFILE := $(SDIR)/GenerateTestCases.py

PY := python3
PYSCRIPT := ./search.py 

all: $(OUTFILE)

# Making the generator file
$(OUTFILE): $(TESTFILES)
	$(PY) $(PYSCRIPT) $(TESTCASEDIR) $(PREFIX) $^ > $@

clean:
	$(RM) $(OUTFILE)

re: clean
	$(MAKE) all
