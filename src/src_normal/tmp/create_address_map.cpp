#include "config/Config.hpp"
#include "create_address_map.hpp"
#include <limits>

namespace testing
{

Config::ip_host_pair createAddress()
{
	return Config::ip_host_pair("127.0.0.1", 8080);
}

Config::address_map* createAddressMap()
{
	Config::address_map* map = new Config::address_map;

	(*map)[createAddress()] = _detail_::createServerBlockVector();
	return map;
}

	namespace _detail_
	{
	
	Config::server_block_vector createServerBlockVector()
	{
		Config::server_block_vector blocks;

		blocks.push_back(createServerBlock());
		return blocks;
	}

	ServerBlock* createServerBlock()
	{
		ServerBlock* block = new ServerBlock;

		block->_client_body_size = std::numeric_limits<std::size_t>::max();
		block->_locations.push_back(createLocationBlock());
		block->_server_names.push_back("localhost");
		block->_server_names.push_back("127.0.0.1");

		return block;
	}

	LocationBlock* createLocationBlock()
	{
		LocationBlock* location = new LocationBlock;

		location->_allowed_methods.push_back("GET");
		location->_allowed_methods.push_back("POST");
		location->_allowed_methods.push_back("DELETE");
		location->_autoindex_status = true;
	#ifdef __linux__
		location->_cgi.push_back(std::make_pair(".py", "/usr/bin/python3"));
	#else
		location->_cgi.push_back(std::make_pair(".py", "/usr/bin/python3"));
	#endif /* __linux__ */

		location->_index.push_back("index.html");
		location->_location_flag = NONE;
		location->_path = "/";
		location->_root = "./path_sample";

		return location;
	}

	}

}
