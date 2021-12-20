ifndef SRC_FILES_MK
SRC_FILES_MK = 1

NAME = a.out
CATCH_NAME = catch.out
FSANITIZE_NAME = fsanitize.out
DEBUG_NAME = debug.out
EXECS = $(NAME) $(CATCH_NAME) $(FSANITIZE_NAME) $(DEBUG_NAME)

SDIR = src
DDIR = dep$(DSUBDIR)
ODIR = obj$(OSUBDIR)
IDIR = includes

SRC_SUBDIR = src_normal

ifdef USING_CATCH
	NAME = $(CATCH_NAME)
	CATCH_SUBDIR = src_catch
	OSUBDIR = /obj_catch
	DSUBDIR = /dep_catch
endif # USING_CATCH

ifdef DEBUG
	NAME = $(DEBUG_NAME)
	OSUBDIR = /obj_debug
	DSUBDIR = /dep_debug
endif # DEBUG

ifdef FSANITIZE
	NAME = $(FSANITIZE_NAME)
	OSUBDIR = /obj_fsanitize
	DSUBDIR = /dep_fsanitize
endif # FSANITIZE

SRC := $(shell find $(SDIR)/$(SRC_SUBDIR) -type f -name "*.cpp")
ifdef USING_CATCH
	SRC += $(shell find $(SDIR)/$(CATCH_SUBDIR) -type f -name "*.cpp")
endif # USING_CATCH
OBJ := $(patsubst $(SDIR)/%.cpp,$(ODIR)/%.o,$(SRC))
DEP := $(patsubst $(SDIR)/%.cpp,$(DDIR)/%.d,$(SRC))

endif # SRC_FILES_MK
