#pragma once
#include <string>

struct Request;

struct ResponseInfo;

class FdTable;

class RequestExecutor
{
	public:
		enum Status
		{
			NOT_COMPLETE,
			BAD_REQUEST,
			CONTINUE,
			REDIRECT,
			AUTO_INDEX_ON,
			HANDLER_ERROR,
			TARGET_NOT_FOUND,
			TARGET_IS_DIRECTORY
		};

	public:
		RequestExecutor();
	private:
		RequestExecutor(RequestExecutor const & src);
		RequestExecutor&	operator=(RequestExecutor const & rhs);

	public:
		void	executeRequest(FdTable & fd_table, Request & request, ResponseInfo & response);
	private:
		void	markStatus(Status status, int status_code);
		bool	isRequestComplete(Request const & request);
		bool	isLocationResolved(Request const & request);
		bool	isRequestTargetValid(std::string const & target);
		std::string	getEffectiveRequestURI(Request const & request);
		void	setAbsoluteFilePath(Request const & request, ResponseInfo & response);

	private:
		int 		_status_code;
		Status		_status;

};
