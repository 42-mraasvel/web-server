#pragma once

# include "config/Config.hpp"

namespace testing
{

Config::ip_host_pair createAddress();
Config::address_map* createAddressMap();

	namespace _detail_
	{

	Config::server_block_vector createServerBlockVector();
	SmartPointer<ServerBlock> createServerBlock();
	SmartPointer<LocationBlock> createLocationBlock();

	}

} // namespace testing
