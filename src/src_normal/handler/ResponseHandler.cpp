#include "ResponseHandler.hpp"
#include "settings.hpp"
#include "utility/utility.hpp"
#include "utility/status_codes.hpp"
#include "fd/AFdInfo.hpp"
#include "request/Request.hpp"
#include "request/RequestExecutor.hpp"

#include <unistd.h>
#include <cstdlib>
#include <fcntl.h>
#include <errno.h>
#include <iostream>
#include <time.h>
#include <algorithm>

ResponseHandler::ResponseHandler() {}

void	ResponseHandler::processRequest(FdTable & fd_table, Request & request)
{
	ResponsePointer	r = ResponsePointer(new Response(request));
	_response_queue.push_back(r);
	RequestExecutor	executor;
	executor.executeRequest(fd_table, request, *r);
}

void	ResponseHandler::updateResponseQueue(FdTable & fd_table)
{
	for (ResponseQueue::iterator it = _response_queue.begin(); it != _response_queue.end(); ++it)
	{
		_updator.update(fd_table, *(*it));
	}
}

ResponseHandler::ResponsePointer	ResponseHandler::getNextResponse()
{
	if (_response_queue.empty())
	{
		return NULL;
	}
	ResponsePointer	r = _response_queue.front();
	return r;
}

void		ResponseHandler::popQueue()
{
	_response_queue.pop_front();
}

bool		ResponseHandler::isResponseQueueEmpty() const
{
	return _response_queue.empty();
}

bool		ResponseHandler::canExecuteRequest() const
{
	return _response_queue.size() < RESPONSE_QUEUE_MAX_SIZE;
}

void		ResponseHandler::clear()
{
	_response_queue.clear();
}
