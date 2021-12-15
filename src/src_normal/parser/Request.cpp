#include "Request.hpp"
#include "color.hpp"
#include <cstdio> // RM, REMOVE: FOR PRINT/DEBUGGING

/*
	RequestStatus	status;

	MethodType		type;
	std::string		request_target;
	int				major_version;
	int				minor_version;
	header_field_t	header_fields;
	std::string		message_body;
*/
Request::Request()
{
	init();
}

Request::Request(Address client, Address interface)
: address(client), interface_addr(interface)
{
	init();
}

void Request::init()
{
	status = Request::READING;
	status_code = 0;
	close_connection = false;
	method = Method::OTHER;
	major_version = -1;
	minor_version = -1;
}

/*
Debugging
Move to become part of REQUEST
*/

Method::Type Request::getMethodType(std::string const & s)
{
	static const std::string types[] = {
		"GET",
		"POST",
		"DELETE"
	};

	for (int i = Method::GET; i < Method::OTHER; ++i)
	{
		if (types[i] == s)
		{
			return static_cast<Method::Type>(i);
		}
	}
	return Method::OTHER;
}

std::string Request::getMethodString() const
{
	switch (method)
	{
		case Method::GET:
			return "GET";
		case Method::POST:
			return "POST";
		case Method::DELETE:
			return "DELETE";
		case Method::OTHER:
			break;
	}
	return "OTHER";
}

std::string Request::getStatusString() const
{
	switch (status)
	{
		case READING:
			return "READING";
		case EXPECT:
			return "EXPECT";
		case COMPLETE:
			return "COMPLETE";
		case BAD_REQUEST:
			return "BAD_REQUEST";
		case TIME_OUT_REQUEST:
			return "TIME_OUT_REQUEST";
	}
	return "???";
}

std::string Request::getProtocolString() const
{
	if (minor_version == 1)
	{
		return "HTTP/1.1";
	}
	else
	{
		return "HTTP/1.0";
	}
}

void Request::printBodyBytes() const
{
	for (std::size_t i = 0; i < message_body.size(); ++i) {
		if (i != 0) {
			printf(" ");
		}
		printf("%d", message_body[i]);
	}
	printf("\n");
}

template <typename C>
void printVector(const std::string& prefix, const C& c)
{
	printf("%s\n", prefix.c_str());
	for (std::size_t i = 0; i < c.size(); ++i)
	{
		printf("    [%s]\n", c[i].c_str());
	}
}

static void	printConfigResult(ConfigInfo::ConfigResult const & result)
{
	std::string	string;
	switch (result)
	{
		case ConfigInfo::NOT_FOUND:
			string = "NOT_FOUND";
			break ;
		case ConfigInfo::AUTO_INDEX_ON:
			string = "AUTO_INDEX_ON";
			break ;
		case ConfigInfo::REDIRECT:
			string = "REDIRECT";
			break ;
		default:
			string = "LOCATION_RESOLVED";
	}
	printf("Resolved result: %s\n", string.c_str());
}

static void printConfigInfo(const ConfigInfo& info)
{
	printf(GREEN_BOLD "-- REQUEST CONFIG INFO --" RESET_COLOR "\n");
	
	if (!info.resolved_server)
	{
		printf("No server block resolved.\n");
		return;
	}
	printConfigResult(info.result);
	printf("Client Max Body Size: %lu\n", info.resolved_server->_client_body_size);
	printVector("  -- SERVER NAMES --", info.resolved_server->_server_names);

	if (info.result == ConfigInfo::NOT_FOUND)
	{
		printf("No location block resolved.\n");
		return ;
	}
	else
	{
		printf("Resolved location block is: [%s]\n", info.resolved_location->_path.c_str());
	}
	printf("Resolved Target: %s\n", info.resolved_target.c_str());
	printf("Resolved File Path: %s\n", info.resolved_file_path.c_str());
	printVector("  -- ALLOWED METHODS --", info.resolved_location->_allowed_methods);
	printVector("  -- INDEX --", info.resolved_location->_index);
	printf("  -- CGI --\n");
	for (std::size_t i = 0; i < info.resolved_location->_cgi.size(); ++i)
	{

		printf("    [%s]: [%s]\n",
		info.resolved_location->_cgi[i].first.c_str(),
		info.resolved_location->_cgi[i].second.c_str());
	}

}

void Request::print() const
{
	printf(GREEN_BOLD "-- PARSED REQUEST --" RESET_COLOR "\r\n");
	printf("%s [%s][%s] HTTP/%d.%d\r\n",
		getMethodString().c_str(),
		request_target.c_str(),
		query.c_str(),
		major_version, minor_version);
	printf("Status: %s\n", getStatusString().c_str());
	printf("StatusCode: %d\n", status_code);
	printf("Address: '%s:%d'\n", address.first.c_str(), address.second);
	printf(" %s- Header Field -%s\n", GREEN_BOLD, RESET_COLOR);
	for (header_field_t::const_iterator it = header_fields.begin(); it != header_fields.end(); ++it)
	{
		printf("  %s: %s\r\n", it->first.c_str(), it->second.c_str());
	}
	printf(GREEN_BOLD " - Message Body -" RESET_COLOR "\r\n");
	printf("Body-Size(%lu)\n", message_body.size());
	if (message_body.size() <= 8192) {
		printf("%s\r\n", message_body.c_str());
		printBodyBytes();
	}
	else {
		printf("body too large to print\n");
	}
	printf(GREEN_BOLD " - Other feature -" RESET_COLOR "\r\n");
	if (close_connection)
	{
		printf("Close_connection: yes\n");
	}
	else
	{
		printf("Close_connection: no\n");
	}
	printf(GREEN_BOLD "------------------------" RESET_COLOR "\r\n");
	// printConfigInfo(config_info);
}
