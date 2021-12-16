#pragma once
#include <string>
#include <deque>
#include "Response.hpp"
#include "ResponseUpdator.hpp"

struct	Request;
class	FdTable;

class ResponseHandler
{
	public:
		typedef	SmartPointer< Response >		ResponsePointer;
		typedef std::deque< ResponsePointer >	ResponseQueue;

	public:
		ResponseHandler();
	private:
		ResponseHandler(ResponseHandler const & src);
		ResponseHandler const &	operator=(ResponseHandler const & rhs);

	/* Client::update() */
	public:
		void	processRequest(FdTable & fd_table, Request & request);
		void	updateResponseQueue(FdTable & fd_table);
	private:
		void	updateResponse(FdTable & fd_table, Response & response);

	/* Client::writeEvent() */
	public:
		ResponsePointer	getNextResponse();
		void			popQueue();

	/* utility */
	public:
		bool	isResponseQueueEmpty() const;
		void	clear();

	private:
		ResponseQueue		_response_queue;
		ResponseUpdator		_updator;
};
