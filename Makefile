MK_SETTINGS_DIR := make_settings
DEPENDENCIES := Makefile $(shell find $(MK_SETTINGS_DIR) -type f -name "*.mk")
# DEPENDENCIES := Makefile

include $(MK_SETTINGS_DIR)/src_files.mk
include $(MK_SETTINGS_DIR)/settings.mk
include $(MK_SETTINGS_DIR)/output.mk

all:
	$(MAKE) $(NAME) -j4

# Compilation
$(NAME): $(OBJ)
	$(CXX) -o $@ $(OBJ) $(LFLAGS)
$(OBJ): $(ODIR)/%.o: $(SDIR)/%.cpp $(DEPENDENCIES)
	@mkdir -p $(@D)
	$(CXX) -c -o $@ $< $(CXXFLAGS) $(IFLAGS)
$(DDIR)/%.d: $(SDIR)/%.cpp
	@mkdir -p $(@D)
	$(CXX) $< -MM -MF $@ -MT $(ODIR)/$*.o $(IFLAGS) $(CXXFLAGS)

# Clean up
.PHONY: clean fclean re
clean:
	$(RM) -r $(DDIR)
	$(RM) -r $(ODIR)
fclean: clean
	$(RM) $(EXECS)
re: fclean
	$(MAKE) all

# Catch
.PHONY: catch debug fsanitize
catch:
	$(MAKE) all USING_CATCH=1
debug:
	$(MAKE) all DEBUG=1
fsanitize:
	$(MAKE) all FSANITIZE=1

# Printing
.phony: verbose printall noinfo
verbose:
	$(RM) $(ODIR)/$(OUTPUT_FILE)
	$(MAKE) all PRINT_VERBOSE=1
printall:
	$(RM) $(ODIR)/$(OUTPUT_FILE)
	$(MAKE) all PRINT_VERBOSE=1 PRINT_DEBUG=1
default:
	$(RM) $(ODIR)/$(OUTPUT_FILE)
	$(MAKE) all

-include $(DEP)
