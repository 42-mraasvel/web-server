include make_settings/src_files.mk
include make_settings/settings.mk
include make_settings/fadey.mk

all: $(NAME)

# Compilation
$(NAME): $(FADEY) $(OBJ)
	$(CXX) -o $@ $(OBJ) $(FLINK) -I $(FHDIR) $(LFLAGS)
$(OBJ): $(ODIR)/%.o: $(SDIR)/%.cpp Makefile
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
	$(MAKE) -C $(FADIR) clean
fclean: clean
	$(RM) $(EXECS)
re: fclean
	$(MAKE) all

# Catch
.PHONY: catch debug
catch:
	$(MAKE) all USING_CATCH=1
debug:
	$(MAKE) all DEBUG=1

-include $(DEP)

$(FADEY):
	Make -C $(FADIR)
