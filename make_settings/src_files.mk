ifndef SRC_FILES_MK
SRC_FILES_MK = 1

NAME = IGotUBroA.outIsForNoobs
CATCH_NAME = catch.out
DEBUG_NAME = debug_build.out
EXECS = $(NAME) $(CATCH_NAME) $(DEBUG_NAME)

SDIR = src
ODIR = obj
DDIR = dep
IDIR = includes

SRC_SUBDIR = src_normal

ifdef USING_CATCH
	NAME = $(CATCH_NAME)
	CATCH_SUBDIR = src_catch
	ODIR = obj/obj_catch
	DDIR = dep/dep_catch
endif # USING_CATCH

ifdef DEBUG
	NAME = $(DEBUG_NAME)
	ODIR = obj/obj_debug
	DDIR = dep/dep_debug
endif # DEBUG

SRC := $(shell find $(SDIR)/$(SRC_SUBDIR) -type f -name "*.cpp")
ifdef USING_CATCH
	SRC += $(shell find $(SDIR)/$(CATCH_SUBDIR) -type f -name "*.cpp")
endif # USING_CATCH
OBJ := $(patsubst $(SDIR)/%.cpp,$(ODIR)/%.o,$(SRC))
DEP := $(patsubst $(SDIR)/%.cpp,$(DDIR)/%.d,$(SRC))

endif # SRC_FILES_MK
