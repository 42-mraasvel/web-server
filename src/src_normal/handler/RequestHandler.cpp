#include "RequestHandler.hpp"
#include "settings.hpp"
#include "parser/Request.hpp"

RequestHandler::RequestHandler()
: _request(NULL) {}

RequestHandler::~RequestHandler() {
	while (!_requests.empty())
	{
		Request* x = _requests.front();
		_requests.pop();
		delete x;
	}
}

int RequestHandler::parse(std::string const & buffer)
{
	std::size_t index = 0;
	while (index < buffer.size())
	{
		if (_request == NULL)
		{
			newRequest();
		}
		if (_parser.parse(buffer, index, *_request) == ERR)
		{
			setErrorRequest();
			return ERR;
			break;
		}
		else if (_parser.isComplete())
		{
			_request->status = Request::COMPLETE;
			// _request->print();
			completeRequest();
		}
	}

	return OK;
}

Request* RequestHandler::getNextRequest()
{
	if (_requests.empty())
	{
		return NULL;
	}

	Request* r = _requests.front();
	_requests.pop();
	return r;
}

void RequestHandler::newRequest()
{
	_request = new Request;
}

void RequestHandler::completeRequest()
{
	_requests.push(_request);
	_request = NULL;
	_parser.reset();
}

void RequestHandler::setErrorRequest()
{
	_request->status = Request::BAD_REQUEST;
	_request->status_code = _parser.getStatusCode();
	completeRequest();
}
