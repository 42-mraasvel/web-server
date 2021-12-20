CGI_EXEC_DIR := ./cgi_executable
CGI_EXEC := mrtest_cgi.out
SERVER_ROOT_DIR := ./ServerRoot/cgi/cgi_bin

all:
	$(MAKE) -C $(CGI_EXEC_DIR)
	$(MAKE) $(SERVER_ROOT_DIR)/$(CGI_EXEC)

$(SERVER_ROOT_DIR)/$(CGI_EXEC): $(CGI_EXEC_DIR)/$(CGI_EXEC)
	mkdir -p $(@D)
	cp $< $@

$(CGI_EXEC_DIR)/$(CGI_EXEC):
	$(MAKE) -C $(CGI_EXEC_DIR)
