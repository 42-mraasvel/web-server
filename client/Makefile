# Executable
NAME := client.out

# Directories
SDIR := src
ODIR := obj
DDIR := dep
IDIR := includes
LDIR := libs

# Files
SRC := $(shell find $(SDIR) -name "*.cpp" -type f)
OBJ := $(patsubst $(SDIR)/%.cpp,$(ODIR)/%.o,$(SRC))
DEP := $(patsubst $(SDIR)/%.cpp,$(DDIR)/%.d,$(SRC))
HDF := $(shell find $(IDIR) -name "*.hpp" -type f)

# Compilation
CXX = clang++
IFLAGS = -I$(IDIR) -I.
LFLAGS =
CXXFLAGS = -Wall -Wextra -Werror -std=c++98 -pedantic

ifeq ($(shell uname -s),Linux)
	DFLAGS := -O0 -g -fsanitize=address -fsanitize=leak
else
	DFLAGS := -O0 -g -fsanitize=address
endif

ifdef DEBUG
	CXXFLAGS += $(DFLAGS)
endif

RM = /bin/rm -f

# Rules
.PHONY: all
all: $(NAME)

# Compilation
$(NAME): $(OBJ)
	$(CXX) -o $@ $(OBJ) $(CXXFLAGS) $(LFLAGS)
$(OBJ): $(ODIR)/%.o: $(SDIR)/%.cpp Makefile
	@mkdir -p $(@D)
	$(CXX) -c -o $@ $< $(CXXFLAGS) $(IFLAGS)
$(DDIR)/%.d: $(SDIR)/%.cpp
	@mkdir -p $(@D)
	@$(CXX) $< -MM -MF $@ -MT $(ODIR)/$*.o $(IFLAGS)

# Cleanup
.PHONY: clean fclean re cleanall
clean:
	$(RM) -r $(ODIR)
	$(RM) -r $(DDIR)
fclean: clean
	$(RM) $(NAME)
re: fclean
	$(MAKE) all

# Debug
.PHONY: debug
debug: fclean
	$(MAKE) all DEBUG=1

-include $(DEP)
