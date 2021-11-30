#pragma once
#include "ResponseInfo.hpp"
#include "FileHandler.hpp"
#include "handler/CgiHandler.hpp"

struct Request;

class FdTable;

class ResponseProcessor
{
	public:
		ResponseProcessor(Request const & request);

	/* Client::update() - dispatchRequest */
	public:
		void	executeRequest(FdTable & fd_table, Request & request);
	private:
		void		executeCompleteRequest(FdTable & fd_table, Request & request);
		void			dispatchConfigResult(FdTable & fd_table, Request & request);
		int				checkRequestTarget(Request const & request);
		void				setEffectiveRequestURI(Request const & request, std::string const & resolved_target);
		void				setAbsoluteFilePath(std::string const & resolved_file_path);

	/* Client::update() - updateResponse */
	public:
		void	updateResponse(FdTable & fd_table, std::string & message_body);
	private:
		void		setMessageBody(FdTable & fd_table, std::string & message_body);
		void			processRedirectResponse(std::string & message_body);
		void			processAutoIndex(std::string & message_body);
		bool			isErrorPageRedirected(FdTable & fd_table);
		void			setOtherErrorPage(std::string & message_body);
		void		setEncoding(std::string const & message_body);
		bool			isReadyToBeChunked(std::string const & message_body) const;

	/* utility */
	public:
		bool			isChunked() const;
		ResponseInfo	getInfo() const;
		iHandler*		getHandler() const;
	private:
		void	markComplete(int code);
		void	setCgi();
		void	unsetCgi();
	
	private:
		ResponseInfo	_info;
		iHandler*		_handler;
		FileHandler		_file_handler;
		CgiHandler		_cgi_handler;

};
