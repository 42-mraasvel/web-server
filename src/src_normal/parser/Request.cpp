#include "Request.hpp"

bool case_insensitive_less::operator() (const std::string& x, const std::string& y) const
{
	for (std::size_t i = 0; i < x.size() && i < y.size(); ++i) {
		if (toupper(x[i]) != toupper(y[i])) {
			return toupper(x[i]) < toupper(y[i]);
		}
	}
	return x.size() < y.size();
}
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
: status(Request::READING), type(Request::OTHER), major_version(-1), minor_version(-1) {}
