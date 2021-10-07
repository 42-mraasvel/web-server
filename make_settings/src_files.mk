ifndef SRC_FILES_MK
SRC_FILES_MK = 1

NAME = a.out
CATCH_NAME = catch.exe
EXECS = $(NAME) $(CATCH_NAME)

SDIR = src
ODIR = obj
DDIR = dep
IDIR = includes

SRC_SUBDIR = src_normal

ifdef USING_CATCH
	NAME = $(CATCH_NAME)
	CATCH_SUBDIR = src_catch
	ODIR = obj/obj_catch
endif # USING_CATCH

SRC := $(shell find $(SDIR)/$(SRC_SUBDIR) -type f -name "*.cpp")
ifdef USING_CATCH
	SRC += $(shell find $(SDIR)/$(CATCH_SUBDIR) -type f -name "*.cpp")
endif # USING_CATCH
OBJ := $(patsubst $(SDIR)/%.cpp,$(ODIR)/%.o,$(SRC))
DEP := $(patsubst $(SDIR)/%.cpp,$(DDIR)/%.d,$(SRC))

endif # SRC_FILES_MK
