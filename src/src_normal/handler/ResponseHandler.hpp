#pragma once
#include <string>
#include <deque>
#include "fd/AFdInfo.hpp"
#include "Response.hpp"

struct	Request;

class ResponseHandler
{
	public:
		typedef	SmartPointer< Response >		ResponsePointer;
		typedef std::deque< ResponsePointer >	ResponseQueue;

	/* Client::update() */
	public:
		void	processRequest(FdTable & fd_table, Request & request);
		void	updateResponseQueue(FdTable & fd_table);

	/* Client::writeEvent() */
	public:
		ResponsePointer	getNextResponse();

	/* utility */
	public:
		bool	isResponseQueueEmpty() const;
		bool	canClientWrite() const;
		void	clear();

	private:
		ResponseQueue		_response_queue;
};
