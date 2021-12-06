#pragma once
#include "config/Config.hpp"

struct ConfigInfo
{
	public:

		typedef ConfigServer::server_pointer server_block_pointer;
		typedef ConfigLocation::location_pointer location_block_pointer;

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
		server_block_pointer	resolved_server;
		location_block_pointer	resolved_location;
		std::string		resolved_target;
		std::string		resolved_file_path;
};
