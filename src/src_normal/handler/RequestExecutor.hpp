#pragma once

struct Request;

struct ResponseInfo;

class RequestExecutor
{
	public:
		enum Status
		{
			OK,
			BAD_REQUEST,
			100_CONTINUE,
			REDIRECT,
			AUTO_INDEX_ON,
			HANDLER_ERROR,
			TARGET_ERROR
		};

	public:
		RequestExecutor();
	private:
		RequestExecutor(RequestExecutor const & src);
		RequestExecutor&	operator=(RequestExecutor const & rhs);

	public:
		void	executeRequest(FdTable & fd_table, Request & request);
	private:
		int		dispatchRequest(Request const & request);
		int		dispatchConfigResult(Request const & request);
		int				checkRequestTarget(Request const & request);
		void				setEffectiveRequestURI(Request const & request, std::string const & resolved_target);
		void				setAbsoluteFilePath(std::string const & resolved_file_path);
		void	markStatus(Status status, int status_code);

	private:
		int 	_status_code;
		Status	_status;
		bool	_is_cgi;

};
