#pragma once

# include "parser/Request.hpp"
# include "parser/HeaderField.hpp"
# include "fd/FdTable.hpp"
# include "fd/CgiSender.hpp"
# include "fd/CgiReader.hpp"
# include "cgi/CgiExecutor.hpp"
# include "iHandler.hpp"
# include "utility/Timer.hpp"

# include <vector>
# include <utility>

class CgiHandler: public iHandler
{
	private:
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
	
	private:
		int initializeCgiConnection(int* cgi_fds, FdTable& fd_table, Request& r);
		int initializeCgiReader(int* cgi_fds, FdTable& fd_table);
		int initializeCgiSender(int* cgi_fds, FdTable& fd_table, Request& r);

		void finishCgi(Status status, int code);
		int checkStatusField() const;
		bool skippedHeaderField(std::string const & key) const;

	/* Update Functionality */
		void evaluateReader(std::string & response_body);
		void swapHeader();
		bool isExecutionError() const;
		int getErrorCode() const;

	/* Destruction */
		void destroyFds();

	private:
		Status		_status;
		int			_status_code;
		HeaderField _header;

	private:
		CgiExecutor _executor;
		SmartPointer<CgiSender> _sender;
		SmartPointer<CgiReader> _reader;
		Timer _timer;

		pid_t _cgi_pid;
};
