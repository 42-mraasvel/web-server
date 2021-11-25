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
	CXXFLAGS += -std=c++11 -pedantic
endif # USING_CATCH

# REMOVE
ANNOYING_FLAGS = -Wno-unused -Wno-unused-parameter
CXXFLAGS += $(ANNOYING_FLAGS)

ifeq ($(shell uname),Darwin)
	DFLAGS = -O0 -g3 -fsanitize=address
else
	DFLAGS = -O0 -g3 -fsanitize=address -fsanitize=leak
endif # Darwin

ifdef DEBUG
	CXXFLAGS += -O0 -g3
endif # DEBUG

ifdef FSANITIZE
	CXXFLAGS += $(DFLAGS)
	LFLAGS += $(DFLAGS)
endif # FSANITIZE

ifndef DEBUG
	ifndef FSANITIZE
		CXXFLAGS += -O3
	endif # FSANITIZE
endif # DEBUG

endif # SETTINGS_MK
