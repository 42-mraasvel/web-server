#include "Request.hpp"
#include "color.hpp"
#include <cstdio> // RM, REMOVE: FOR PRINT/DEBUGGING

/*
	RequestStatus	status;

	MethodType		type;
	std::string		target_resource;
	int				major_version;
	int				minor_version;
	header_field_t	header_fields;
	std::string		message_body;
*/
Request::Request()
: status(Request::READING),
status_code(200),
processed(false),
method(OTHER),
major_version(-1),
minor_version(-1) {}

/*
Debugging
Move to become part of REQUEST
*/

std::string Request::getMethodString() const
{
	switch (method)
	{
		case GET:
			return "GET";
		case POST:
			return "POST";
		case DELETE:
			return "DELETE";
		case OTHER:
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
		case HEADER_COMPLETE:
			return "HEADER_COMPLETE";
		case COMPLETE:
			return "COMPLETE";
		case BAD_REQUEST:
			return "BAD_REQUEST";
	}
	return "???";

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

void Request::print() const
{
	printf(GREEN_BOLD "-- PARSED REQUEST --" RESET_COLOR "\r\n");
	printf("Status: %s\n", getStatusString().c_str());
	printf("%s %s HTTP/%d.%d\r\n",
		getMethodString().c_str(), target_resource.c_str(),
		major_version, minor_version);
	
	for (header_field_t::const_iterator it = header_fields.begin(); it != header_fields.end(); ++it)
	{
		printf("  %s: %s\r\n", it->first.c_str(), it->second.c_str());
	}
	printf(GREEN_BOLD "-- MESSAGE BODY --" RESET_COLOR "\r\n");
	printf("Body-Size(%lu)\n", message_body.size());
	printf("%s\r\n", message_body.c_str());
	printBodyBytes();
}
