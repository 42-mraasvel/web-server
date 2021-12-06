SDIR = src
TESTCASEDIR = $(SDIR)/TestCaseGeneration

TESTFILES := $(shell find $(TESTCASEDIR) -name "*.py" -type f)
OUTFILE := $(SDIR)/GenerateTestCases.py

PY := python3
PYSCRIPT := ./search.py 

generate:
	$(PY) $(PYSCRIPT) $(TESTFILES)
