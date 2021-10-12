ifndef SETTINGS_MK
SETTINGS_MK = 1

CXX = clang++
CXXFLAGS = -Wall -Wextra -Werror
IFLAGS = -I$(IDIR) -I$(SDIR)/$(SRC_SUBDIR)
LFLAGS =

ifdef USING_CATCH
	CXXFLAGS += -std=c++14 -pedantic -DUSING_CATCH
else
	CXXFLAGS += -std=c++98 -pedantic
endif # USING_CATCH

# REMOVE
ANNOYING_FLAGS = -Wno-unused -Wno-unused-parameter
CXXFLAGS += $(ANNOYING_FLAGS)

ifeq ($(shell uname),Darwin)
	DFLAGS = -g3 -fsanitize=address
else
	DFLAGS = -g3 -fsanitize=address -fsanitize=leak
endif # Darwin

ifdef DEBUG
	CXXFLAGS += $(DFLAGS)
	LFLAGS += $(DFLAGS)
else
	CXXFLAGS += -g3 -O0
endif # DEBUG

endif # SETTINGS_MK
