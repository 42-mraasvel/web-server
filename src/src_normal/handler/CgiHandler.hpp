#pragma once

# include "parser/Request.hpp"
# include "parser/HeaderField.hpp"
# include "fd/FdTable.hpp"
# include "fd/CgiSender.hpp"
# include "fd/CgiReader.hpp"
# include "iHandler.hpp"
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

	private:
		CgiHandler(CgiHandler const & rhs);
		CgiHandler & operator=(CgiHandler const & rhs);

	public:
		CgiHandler();
		~CgiHandler();

	/* Main Interface Functions */
		static bool isCgi(Request const & request);
		static bool isCgi(std::string const & request_target, CgiVectorType const & cgi);

		int		executeRequest(FdTable& fd_table, Request& request);
		void	update(std::string & response_body);
		void	exceptionEvent();
		bool	isComplete() const;
		bool	isError() const;
		int		getStatusCode() const;
		void	setSpecificHeaderField(HeaderField & header_field, bool content_type_fixed);

	/* Debugging */
	public:
		void print() const;
	
	private:

		// void splitRequestTarget(std::string const & request_target, CgiVectorType const & cgi);
		bool scriptCanBeExecuted();

		void setInfo(ConfigInfo const & info);
		void resolveCgiScript(std::string const target, CgiVectorType const & cgi);
		void generateMetaVariables(const Request& request);
		void metaVariableContent(const Request& request);
		void metaVariablePathInfo(const Request& request);
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
		void evaluateReader(std::string & response_body);
		bool isExecutionError() const;
		int getErrorCode() const;
		int cleanCgi();
		int killCgi(int* status);

	/* Destruction */

		void destroyFds();
		void clear();

	private:
		Status		_status;
		int			_status_code;
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
