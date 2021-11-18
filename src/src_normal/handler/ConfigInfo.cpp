#include "ConfigInfo.hpp"

ConfigInfo::ConfigInfo(): resolved_server(NULL), resolved_location(NULL) {}

ConfigInfo& ConfigInfo::operator=(ConfigInfo const & rhs)
{
	if (this != &rhs)
	{
		this->result = rhs.result;
		this->resolved_server = rhs.resolved_server;
		this->resolved_location = rhs.resolved_location;
		this->resolved_target = rhs.resolved_target;
		this->resolved_file_path = rhs.resolved_file_path;
	}
	return *this;
}
