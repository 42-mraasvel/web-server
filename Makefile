include make_settings/src_files.mk
include make_settings/settings.mk

all: $(NAME)

# Compilation
$(NAME): $(OBJ)
	$(CXX) -o $@ $(OBJ) $(LFLAGS)
$(OBJ): $(ODIR)/%.o: $(SDIR)/%.cpp
	@mkdir -p $(@D)
	$(CXX) -c -o $@ $< $(CXXFLAGS) $(IFLAGS)
$(DDIR)/%.d: $(SDIR)/%.cpp
	@mkdir -p $(@D)
	$(CXX) $< -MM -MF $@ -MT $(ODIR)/$*.o $(IFLAGS)

# Clean up
.PHONY: clean fclean re
clean:
	$(RM) -r $(DDIR)
	$(RM) -r $(ODIR)
fclean:
	$(RM) $(EXECS)
re: fclean
	$(MAKE) all

# Catch
.PHONY: catch
catch:
	$(MAKE) all USING_CATCH=1

-include $(DEP)
