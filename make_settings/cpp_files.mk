ifndef CPP_FILES_MK
CPP_FILES_MK=1

SRC :=	src/src_normal/response/ResponseGenerator.cpp\
		src/src_normal/response/Response.cpp\
		src/src_normal/response/ResponseUpdator.cpp\
		src/src_normal/utility/list_directory.cpp\
		src/src_normal/utility/print_body.cpp\
		src/src_normal/utility/is_functions.cpp\
		src/src_normal/utility/make_non_blocking.cpp\
		src/src_normal/utility/Timer.cpp\
		src/src_normal/utility/ft_itoa.cpp\
		src/src_normal/utility/strtoul.cpp\
		src/src_normal/utility/ipvalidation.cpp\
		src/src_normal/utility/close_pipe.cpp\
		src/src_normal/utility/Output.cpp\
		src/src_normal/utility/status_codes.cpp\
		src/src_normal/utility/string_ends_with.cpp\
		src/src_normal/utility/addition_overflow.cpp\
		src/src_normal/utility/HeaderField.cpp\
		src/src_normal/utility/SmartPointer.cpp\
		src/src_normal/utility/error_msg.cpp\
		src/src_normal/utility/to_lower.cpp\
		src/src_normal/utility/case_insensitive_compare.cpp\
		src/src_normal/utility/string_split.cpp\
		src/src_normal/utility/get_envp.cpp\
		src/src_normal/utility/trim_port.cpp\
		src/src_normal/utility/find_limit.cpp\
		src/src_normal/utility/strtol.cpp\
		src/src_normal/utility/basename.cpp\
		src/src_normal/utility/file_management.cpp\
		src/src_normal/utility/MediaType.cpp\
		src/src_normal/cgi/CgiExecutor.cpp\
		src/src_normal/cgi/CgiResponseParser.cpp\
		src/src_normal/fd/Connection.cpp\
		src/src_normal/fd/CgiSender.cpp\
		src/src_normal/fd/CgiReader.cpp\
		src/src_normal/fd/FdTable.cpp\
		src/src_normal/fd/File.cpp\
		src/src_normal/fd/Server.cpp\
		src/src_normal/fd/AFdInfo.cpp\
		src/src_normal/config/ConfigResolver.cpp\
		src/src_normal/config/ConfigServer.cpp\
		src/src_normal/config/Config.cpp\
		src/src_normal/config/debug_print.cpp\
		src/src_normal/config/ConfigInfo.cpp\
		src/src_normal/config/ConfigLocation.cpp\
		src/src_normal/webserver/Webserver.cpp\
		src/src_normal/main.cpp\
		src/src_normal/request/RequestHeaderProcessor.cpp\
		src/src_normal/request/RequestValidator.cpp\
		src/src_normal/request/Request.cpp\
		src/src_normal/request/RequestExecutor.cpp\
		src/src_normal/parser/HttpRequestParser.cpp\
		src/src_normal/parser/ChunkedParser.cpp\
		src/src_normal/parser/HeaderFieldParser.cpp\
		src/src_normal/parser/ContentParser.cpp\
		src/src_normal/parser/RequestLineParser.cpp\
		src/src_normal/parser/ParserUtils.cpp\
		src/src_normal/handler/FileHandler.cpp\
		src/src_normal/handler/CgiHandler.cpp\
		src/src_normal/handler/ResponseHandler.cpp\
		src/src_normal/handler/RequestHandler.cpp

ifdef USING_CATCH

	SRC +=	src/src_catch/catch_requestheaderprocessor.cpp\
			src/src_catch/catch_main.cpp\
			src/src_catch/catch_utility_tests.cpp\
			src/src_catch/catch.cpp\
			src/src_catch/catch_cgi_tests.cpp\
			src/src_catch/catch_requestline_parser.cpp\
			src/src_catch/catch_parser.cpp\
			src/src_catch/catch_cgi_parser.cpp\
			src/src_catch/catch_resolution.cpp\
			src/src_catch/catch_requestparser_tests.cpp\
			src/src_catch/tmp/create_address_map.cpp\
			src/src_catch/catch_headerfield_parsing.cpp\
			src/src_catch/catch_config.cpp\
			src/src_catch/example_test.cpp

endif # USING_CATCH

endif # CPP_FILES_MK
