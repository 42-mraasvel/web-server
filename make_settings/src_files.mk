ifndef SRC_FILES_MK
SRC_FILES_MK = 1

NAME = a.out
CATCH_NAME = catch.exe
EXECS = $(NAME) $(CATCH_NAME)

SDIR = src
ODIR = obj
DDIR = dep
IDIR = includes

ifdef USING_CATCH
	SRC_SUBDIR = src_catch
	NAME = $(CATCH_NAME)
else
	SRC_SUBDIR = src_normal
endif # USING_CATCH

SRC := $(shell find $(SDIR)/$(SRC_SUBDIR) -type f -name "*.cpp")
OBJ := $(patsubst $(SDIR)/%.cpp,$(ODIR)/%.o,$(SRC))
DEP := $(patsubst $(SDIR)/%.cpp,$(DDIR)/%.d,$(SRC))

endif # SRC_FILES_MK
