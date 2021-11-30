#include "Response.hpp"
#include "settings.hpp"
#include "utility/utility.hpp"
#include "fd/FdTable.hpp"
#include "parser/Request.hpp"

Response::Response(Request const & request):
_processor(request)
{}

/******************************/
/****** Client::update() ******/
/******************************/

void	Response::executeRequest(FdTable & fd_table, Request & request)
{
	_processor.executeRequest(fd_table, request);
}

void	Response::update(FdTable & fd_table)
{
	_processor.updateResponse(fd_table, _generator.message_body);
}


/*********************************/
/****** Client::writeEvent() *****/
/*********************************/

void	Response::generateResponse(std::string & append_to)
{
	_generator.generateResponse(_processor.getInfo(), _processor.getHandler());
	_generator.appendString(append_to);
}

/******************************/
/****** utility - public ******/
/******************************/

bool	Response::isReadyToWrite() const
{
	if (_processor.isChunked())
	{
		return !_generator.message_body.empty() || _processor.getHandler()->isReadyToWrite();
	}
	return false;
}

bool	Response::isComplete() const
{
	return _processor.getInfo().status == ResponseInfo::COMPLETE;
}

Method::Type	Response::getMethod() const
{
	return _processor.getInfo().method;
}

bool			Response::getCloseConnectionFlag() const
{
	return _processor.getInfo().close_connection;
}

