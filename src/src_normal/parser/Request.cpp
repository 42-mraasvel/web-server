#include "Request.hpp"
#include "color.hpp"
#include "settings.hpp"

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
			PRINT_DEBUG << ' ';
		}
		PRINT_DEBUG << static_cast<int> (message_body[i]);
	}
	PRINT_DEBUG << std::endl;
}

template <typename C>
void printVector(const std::string& prefix, const C& c)
{
	PRINT_DEBUG << prefix << std::endl;
	for (std::size_t i = 0; i < c.size(); ++i)
	{
		PRINT_DEBUG << "    [" << c[i] << "]" << std::endl;
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
			break;
	}
	PRINT_DEBUG << "Resolved result: " << string << std::endl;
}

static void printConfigInfo(const ConfigInfo& info)
{
	PRINT_DEBUG << GREEN_BOLD "-- Request Config Info --" RESET_COLOR << std::endl;
	
	if (!info.resolved_server)
	{
		PRINT_DEBUG << "No server block resolved" << std::endl;
		return;
	}
	printConfigResult(info.result);
	PRINT_DEBUG << "Client Max Body Size: [" << info.resolved_server->_client_body_size << "]" << std::endl;
	printVector("  -- SERVER NAMES --", info.resolved_server->_server_names);

	if (info.result == ConfigInfo::NOT_FOUND)
	{
		PRINT_DEBUG << "No locatino block resolved" << std::endl;
		return ;
	}
	PRINT_DEBUG << "Resolved location block: [" << info.resolved_location->_path << "]" << std::endl;
	PRINT_DEBUG << "Resolved target: " << info.resolved_target << std::endl;
	PRINT_DEBUG << "Resolved file path: " << info.resolved_file_path << std::endl;
	printVector("  -- ALLOWED METHODS --", info.resolved_location->_allowed_methods);
	printVector("  -- INDEX --", info.resolved_location->_index);
	PRINT_DEBUG << "  -- CGI -- " << std::endl;
	for (std::size_t i = 0; i < info.resolved_location->_cgi.size(); ++i)
	{
		PRINT_DEBUG << "    [" \
			<< info.resolved_location->_cgi[i].first  << "]: [" \
			<< info.resolved_location->_cgi[i].second << "]" << std::endl;
	}
}

void Request::print() const
{
	PRINT_DEBUG << GREEN_BOLD "-- Parsed Request --" RESET_COLOR << std::endl;
	PRINT_DEBUG << getMethodString() << " [" << request_target << "][" << query << "] HTTP/" \
		<< major_version << "." << minor_version << std::endl;
	PRINT_DEBUG << "Status: " << getStatusString() << std::endl;
	PRINT_DEBUG << "StatusCode: " << status_code << std::endl;
	PRINT_DEBUG << "Address: " << address.first << ":" << address.second << std::endl;
	header_fields.print();

	PRINT_DEBUG << GREEN_BOLD " - Message Body -" RESET_COLOR << std::endl;
	PRINT_DEBUG << "Body-Size(" << message_body.size() << ")" << std::endl;

	if (message_body.size() <= MAX_HEADER_SIZE)
	{
		PRINT_DEBUG << message_body << std::endl;
		printBodyBytes();
	}
	else
	{
		PRINT_DEBUG << "Body too large to print" << std::endl;
	}

	PRINT_DEBUG << GREEN_BOLD " - Other Features -" RESET_COLOR << std::endl;

	PRINT_DEBUG << "Close Connection: " << (close_connection ? "yes" : "no") << std::endl;
	PRINT_DEBUG << GREEN_BOLD "------------------------" RESET_COLOR << std::endl;
	// printConfigInfo(config_info);
}
