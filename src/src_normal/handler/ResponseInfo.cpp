#include "ResponseInfo.hpp"
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

ResponseInfo::ResponseInfo(Request const & request):
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
handler(&file_handler)
{}

