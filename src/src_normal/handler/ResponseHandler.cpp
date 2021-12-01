#include "ResponseHandler.hpp"
#include "settings.hpp"
#include "utility/utility.hpp"
#include "utility/status_codes.hpp"
#include "parser/Request.hpp"
#include <unistd.h>
#include <cstdlib>
#include <fcntl.h>
#include <errno.h>
#include <iostream>
#include <time.h>
#include <algorithm>

ResponseHandler::ResponseHandler():
_response(NULL)
{}

/*********************************************/
/****** (Client::update) execute request *****/
/*********************************************/

void	ResponseHandler::processRequest(FdTable & fd_table, Request & request)
{
	ResponsePointer	r = ResponsePointer(new Response(request));
	_response_queue.push_back(r);
	r->executeRequest(fd_table, request);
}

/*******************************************/
/****** Client::update() update queue ******/
/*******************************************/

void	ResponseHandler::updateResponseQueue(FdTable & fd_table)
{
	for (ResponseQueue::iterator it = _response_queue.begin(); it != _response_queue.end(); ++it)
	{
		(*it)->update(fd_table);
	}
}

/****************************************************/
/****** (Client::writeEvent) generate response ******/
/***************************************************/

ResponseHandler::ResponsePointer	ResponseHandler::getNextResponse()
{
	if (_response_queue.empty())
	{
		return NULL;
	}
	ResponsePointer	r = _response_queue.front();
	return r;
}

void	ResponseHandler::popQueue()
{
	_response_queue.pop_front();
}

/******************************/
/****** utility - public ******/
/******************************/

bool	ResponseHandler::isResponseQueueEmpty() const
{
	return _response_queue.empty();
}

bool	ResponseHandler::canClientWrite() const
{
	return !_response_queue.empty()
			&& (_response_queue.front()->isComplete()
				|| _response_queue.front()->isReadyToWrite());
}

void	ResponseHandler::clear()
{
	_response_queue.clear();
}
