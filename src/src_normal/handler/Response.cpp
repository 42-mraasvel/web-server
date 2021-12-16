#include "Response.hpp"
#include "parser/Request.hpp"

static std::string	setHttpVersion(int minor_version)
{
	if (minor_version == 0)
	{
		return "HTTP/1.0";
	}
	else
	{
		return "HTTP/1.1";
	}
}

Response::Response(Request const & request):
method(request.method),
request_target(request.request_target),
config_info(request.config_info),
http_version(setHttpVersion(request.minor_version)),
close_connection(request.close_connection),
status(START),
status_code(0),
is_cgi(false),
error_page_attempted(false),
encoding(UNDEFINED),
content_type_fixed(false),
handler(&file_handler),
header_part_set(false)
{}

void	Response::markComplete(int new_status_code)
{
	status = COMPLETE;
	status_code = new_status_code;
}

void	Response::markSpecial(int new_status_code)
{
	status = SPECIAL;
	status_code = new_status_code;
}

bool	Response::isFinished() const
{
	return status == SPECIAL || status == COMPLETE;
}

void	Response::setCgi()
{
	is_cgi = true;
	handler = &cgi_handler;
}

void	Response::unsetCgi()
{
	is_cgi = false;
	handler = &file_handler;
}

void	Response::resetErrorPageRedirection()
{
	status = Response::START;
	encoding = Response::UNDEFINED;
	error_page_attempted = true;
	unsetCgi();
}
