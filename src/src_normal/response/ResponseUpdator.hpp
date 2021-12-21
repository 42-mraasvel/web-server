#pragma once
#include "ResponseGenerator.hpp"

struct	Response;
class	FdTable;

class ResponseUpdator
{
	public:
		ResponseUpdator();
	private:
		ResponseUpdator(ResponseUpdator const & src);
		ResponseUpdator&	operator=(ResponseUpdator const & rhs);

	public:
		void	update(FdTable & fd_table, Response & response);
	private:
		void	updateHandler(Response & response);
		void	setSpecialMessageBody(FdTable & fd_table, Response & response);
		void		processRedirectResponse(Response & response);
		void		processAutoIndex(Response & response);
		bool		isErrorPageRedirected(FdTable & fd_table, Response & response);
		void		setOtherErrorPage(Response & response);

	private:
		ResponseGenerator	_generator;
};
