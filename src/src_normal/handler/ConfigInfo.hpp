#pragma once
#include "config/Config.hpp"

struct ConfigInfo
{
	public:
		enum ConfigResult
		{
			LOCATION_RESOLVED,
			REDIRECT,
			AUTO_INDEX_ON,
			NOT_FOUND
		};

		ConfigInfo();
		ConfigInfo(ConfigInfo const & src);
		ConfigInfo& operator=(ConfigInfo const & rhs);

		ConfigResult	result;
		ServerBlock*	resolved_server;
		LocationBlock*	resolved_location;
		std::string		resolved_target;
		std::string		resolved_file_path;

};
