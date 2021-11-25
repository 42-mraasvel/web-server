#pragma once

# include "parser/Request.hpp"
# include "parser/HeaderField.hpp"
# include "fd/FdTable.hpp"
# include "CgiSender.hpp"
# include "CgiReader.hpp"
# include "handler/iHandler.hpp"
# include "utility/Timer.hpp"

# include <vector>
# include <utility>

class CgiHandler: public iHandler
{
	private:
		typedef std::pair<std::string, std::string> MetaVariableType;
		typedef std::vector<MetaVariableType> MetaVariableContainerType;
		typedef std::vector<std::pair<std::string, std::string> > CgiVectorType;

	public:
		enum Status {
			INACTIVE,
			INCOMPLETE,
			ERROR,
			COMPLETE
		};
	public:
		CgiHandler();
		~CgiHandler();

	/* Main Interface Functions */
		static bool isCgi(Request const & request);
		static bool isCgi(std::string const & request_target, CgiVectorType const & cgi);
		static void resolveCgiTarget(std::string const & target, CgiVectorType const & cgi,
								ConfigInfo & info);

		int		executeRequest(FdTable& fd_table, Request& request);
		void	update();
		bool	isComplete() const;
		bool	isError() const;
		void	setMessageBody(std::string & response_body);
		int		getStatusCode() const;
		bool	isReadyToWrite() const;
		void	setSpecificHeaderField(HeaderField & header_field);

	/* Debugging */
	public:
		void print() const;
	
	private:

		// void splitRequestTarget(std::string const & request_target, CgiVectorType const & cgi);
		bool scriptCanBeExecuted();

		void setInfo(ConfigInfo const & info);
		void generateMetaVariables(const Request& request);
		void metaVariableContent(const Request& request);
		void metaVariableHeader(const Request& request);
		MetaVariableType convertFieldToMeta(const std::string& key, const std::string& value) const;


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

		void finishCgi(Status status, int code);
		int checkStatusField() const;
		bool skippedHeaderField(std::string const & key) const;

	/* Update Functionality */
		bool isExecutionError() const;
		int cleanCgi();
		int killCgi(int* status);

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
		SmartPointer<CgiSender> _sender;
		SmartPointer<CgiReader> _reader;
		Timer _timer;

		pid_t _cgi_pid;
};
