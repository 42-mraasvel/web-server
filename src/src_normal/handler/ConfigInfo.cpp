#include "ConfigInfo.hpp"

ConfigInfo::ConfigInfo(): result(ConfigInfo::NOT_FOUND), resolved_server(NULL), resolved_location(NULL) {}

ConfigInfo::ConfigInfo(ConfigInfo const & src)
{
	*this = src;
}

ConfigInfo& ConfigInfo::operator=(ConfigInfo const & rhs)
{
	if (this != &rhs)
	{
		this->result = rhs.result;
		this->resolved_server = rhs.resolved_server;
		this->resolved_location = rhs.resolved_location;
		this->resolved_target = rhs.resolved_target;
		this->resolved_file_path = rhs.resolved_file_path;
		this->resolved_cgi_script = rhs.resolved_cgi_script;
		this->resolved_path_info = rhs.resolved_path_info;
	}
	return *this;
}
