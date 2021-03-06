ifndef SETTINGS_MK
SETTINGS_MK = 1

CXX = clang++
CXXFLAGS = -Wall -Wextra -Werror
IFLAGS = -I$(IDIR) -I$(SDIR)/$(SRC_SUBDIR)
LFLAGS =

ifdef USING_CATCH
	CXXFLAGS += -std=c++17 -pedantic -DUSING_CATCH
else
	CXXFLAGS += -std=c++98 -pedantic
endif # USING_CATCH

ifeq ($(shell uname),Darwin)
	DFLAGS = -O0 -g3 -fsanitize=address
else
	DFLAGS = -O0 -g3 -fsanitize=address -fsanitize=leak
endif # Darwin

ifdef DEBUG
	CXXFLAGS += -O0 -g3 -DLEAK_CHECK
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

ifdef PRINT_VERBOSE
	CXXFLAGS += -DPRINT_INFO_FLAG
endif # PRINT_VERBOSE
ifdef PRINT_DEBUG
	CXXFLAGS += -DPRINT_DEBUG_FLAG
endif # PRINT_DEBUG

endif # SETTINGS_MK
