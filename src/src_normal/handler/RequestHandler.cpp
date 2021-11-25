#include "RequestHandler.hpp"
#include "settings.hpp"
#include "parser/Request.hpp"

RequestHandler::RequestHandler(AddressType client, AddressType interface, MapType const * config_map)
: _client_addr(client),
_interface_addr(interface),
_request(NULL),
_parser(config_map) {}

RequestHandler::~RequestHandler() {
	while (!_requests.empty())
	{
		RequestPointer x = _requests.front();
		_requests.pop();
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
		_parser.parse(buffer, index, *_request);
		if (_parser.isError())
		{
			setErrorRequest();
			return ERR;
		}
		else if (_parser.isComplete())
		{
			_request->status = Request::COMPLETE;
			completeRequest();
		}
	}

	if (_request && isContinueResponse(*_request))
	{
		newContinueRequest();
	}

	return OK;
}

RequestHandler::RequestPointer RequestHandler::getNextRequest()
{
	if (_requests.empty())
	{
		return NULL;
	}

	RequestPointer r = _requests.front();
	_requests.pop();
	return r;
}

bool RequestHandler::isNextRequestSafe() const
{
	return _requests.empty()
		|| _requests.front()->method == Method::GET;
}

void RequestHandler::newRequest()
{
	_request = RequestPointer(new Request(_client_addr, _interface_addr));
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

/*
100: continue
*/

bool RequestHandler::isContinueResponse(Request const & request) const
{
	return request.header_fields.contains("expect")
			&& request.minor_version == 1
			&& request.header_fields.contains("content-length")
			&& !(request.header_fields.find("content-length")->second.empty())
			&& request.message_body.empty();
}

void RequestHandler::newContinueRequest()
{
	RequestPointer cont = RequestPointer(new Request(_client_addr, _interface_addr));
	cont->config_info = _request->config_info;
	cont->status = Request::EXPECT;
	_requests.push(cont);
}
