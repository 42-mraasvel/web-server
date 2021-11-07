#pragma once

# include "parser/Request.hpp"
# include "parser/HeaderField.hpp"
# include "fd/FdTable.hpp"
# include "CgiSender.hpp"
# include "CgiReader.hpp"

# include <vector>
# include <utility>

class CgiHandler
{
	private:
		typedef std::pair<std::string, std::string> MetaVariableType;
		typedef std::vector<MetaVariableType> MetaVariableContainerType;

	public:
		enum Status {
			INACTIVE,
			INCOMPLETE,
			COMPLETE
		};
	public:
		CgiHandler();
		~CgiHandler();

		bool isCgi(const Request& request);
		int executeRequest(FdTable& fd_table, Request& request);
		bool isComplete() const;

		const std::string& getContent() const;
		const HeaderField& getHeaderField() const;
		int getStatusCode() const;
		Status getStatus() const;

		void generateMessageBody(std::string & message_body);

		void clearContent();

	/* Interfacing Functions */
		void setRootDir(std::string const & root);
		bool isChunked(std::string const & http_version) const;
		bool evaluateExecutionError();
		bool evaluateExecutionCompletion();
		void setMessageBody(std::string & response_body);
		bool isReadyToWrite() const;
		void update();

	/* Debugging */
	public:
		void print() const;
	
	private:

		bool scriptCanBeExecuted();
		void generateMetaVariables(const Request& request);
		void metaVariableContent(const Request& request);

		int initializeCgiConnection(int* cgi_fds, FdTable& fd_table, Request& r);
		int initializeCgiReader(int* cgi_fds, FdTable& fd_table);
		int initializeCgiSender(int* cgi_fds, FdTable& fd_table, Request& r);

		int forkCgi(int* cgi_fds, FdTable& fd_table);
		int prepareCgi(int* cgi_fds, FdTable& fd_table) const;
		int closeAll(FdTable& fd_table) const;
		int setEnvironment() const;
		int setRedirection(int* cgi_fds) const;
		int executeChildProcess() const;
		int prepareArguments(char *args[3]) const;

		void finishResponse(Status status, int code);

	/* Update Functionality */

		void checkCgi();
		bool cgiExists() const;
		void cleanCgi();

	/* Destruction */

		void destroyFds();

	private:
		Status		_status;
		int			_status_code;
		std::string _message_body;
		HeaderField _header;

	private:
		std::string _root_dir;
		std::string _script;
		std::string _target;
		MetaVariableContainerType _meta_variables;
		CgiSender* _sender;
		CgiReader* _reader;

		pid_t _cgi_pid;
};
