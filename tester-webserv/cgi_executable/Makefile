# Executable
NAME = a.out
NAME_DEBUG = debug.out
NAME_FSANITIZE = fsanitize.out
EXECS = $(NAME) $(NAME_DEBUG) $(NAME_FSANITIZE)

# Directories
OTHER_DIR := other
SDIR := src
ODIR := $(OTHER_DIR)/obj
DDIR := $(OTHER_DIR)/dep
IDIR := includes
LDIR := libs

# Naming for different FLAGS
ifdef DEBUG
	NAME = $(NAME_DEBUG)
	SUBDIR = debug/
endif # DEBUG
ifdef FSANITIZE
	NAME = $(NAME_FSANITIZE)
	SUBDIR = fsanitize/
endif # FSANITIZE

# Files
SRC := $(shell find $(SDIR) -name "*.cpp" -type f)
OBJ := $(patsubst $(SDIR)/%.cpp,$(ODIR)/$(SUBDIR)%.o,$(SRC))
DEP := $(patsubst $(SDIR)/%.cpp,$(DDIR)/$(SUBDIR)%.d,$(SRC))

# Compilation
CXX = clang++
IFLAGS = -I$(IDIR)
LFLAGS =
CXXFLAGS = -Wall -Wextra -Werror -std=c++11 -pedantic
DFLAGS := -O0 -g3

# Unused Flags
UNUSED_FLAGS := -Wno-unused -Wno-unused-parameter
CXXFLAGS += $(UNUSED_FLAGS)

ifdef FSANITIZE
	DFLAGS += -fsanitize=address
	ifeq ($(shell uname -s),Linux)
		DFLAGS += -fsanitize=leak
	endif # LINUX
endif # FSANITIZE

ifdef DEBUG
	CXXFLAGS += $(DFLAGS)
	LFLAGS += $(DFLAGS)
endif

RM = /bin/rm -f

# Rules
.PHONY: all
all: $(NAME)

# Compilation
$(NAME): $(OBJ)
	$(CXX) -o $@ $(OBJ) $(LFLAGS)
$(OBJ): $(ODIR)/$(SUBDIR)%.o: $(SDIR)/%.cpp Makefile
	@mkdir -p $(@D)
	$(CXX) -c -o $@ $< $(CXXFLAGS) $(IFLAGS)
$(DDIR)/$(SUBDIR)%.d: $(SDIR)/%.cpp
	@mkdir -p $(@D)
	@$(CXX) $< -MM -MF $@ -MT $(ODIR)/$(SUBDIR)$*.o $(CXXFLAGS) $(IFLAGS)

# Cleanup
.PHONY: clean fclean re cleanall
clean:
	$(RM) -r $(OTHER_DIR)
fclean: clean
	$(RM) $(EXECS)
re: fclean
	$(MAKE) all

# Debug
.PHONY: debug
debug:
	$(MAKE) all DEBUG=1
fsanitize:
	$(MAKE) all DEBUG=1 FSANITIZE=1

-include $(DEP)
