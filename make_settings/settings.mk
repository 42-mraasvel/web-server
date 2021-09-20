ifndef SETTINGS_MK
SETTINGS_MK = 1

CXX = clang++
CXXFLAGS = -Wall -Wextra -Werror
IFLAGS = -I$(IDIR)
LFLAGS =

ifdef USING_CATCH
	CXXFLAGS += -std=c++14 -pedantic
else
	CXXFLAGS += -std=c++98 -pedantic
endif # USING_CATCH

ANNOYING_FLAGS = -Wno-unused -Wno-unused-parameter

ifeq ($(shell uname),Darwin)
	DFLAGS = -g3 -fsanitize=address
else
	DFLAGS = -g3 -fsanitize=address -fanitisze=leaks
endif # Darwin

ifdef DEBUG
	CXXFLAGS += $(DFLAGS)
	LFLAGS += $(DFLAGS)
endif # DEBUG

endif # SETTINGS_MK
