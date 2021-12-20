#pragma once
#include <string>

struct Request;

struct Response;

class FdTable;

class RequestExecutor
{
	public:
		enum Status
		{
			START,
			BAD_REQUEST,
			TIME_OUT_REQUEST,
			CONTINUE,
			REDIRECT,
			AUTO_INDEX_ON,
			HANDLER_ERROR,
			TARGET_NOT_FOUND,
			TARGET_FORBIDDEN,
			TARGET_IS_DIRECTORY
		};

	public:
		RequestExecutor();
	private:
		RequestExecutor(RequestExecutor const & src);
		RequestExecutor&	operator=(RequestExecutor const & rhs);

	public:
		void	executeRequest(FdTable & fd_table, Request & request, Response & response);
	private:
		void	markStatus(Status status, int status_code);
		bool	isRequestComplete(Request const & request);
		void	determineIsCgi(Request & request, Response & response);
		bool	isLocationResolved(Request const & request);
		void	checkAutoIndexDirectory(Request const & request);
		bool	isRequestTargetValid(Response const & response, std::string const & request_target);
		std::string	getEffectiveRequestURI(Request const & request);
		void	setAbsoluteFilePath(Request const & request, Response & response);

	private:
		Status		_status;
		int 		_status_code;

};
