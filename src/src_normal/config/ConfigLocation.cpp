#include "ConfigLocation.hpp"
#include <iostream>
#include "settings.hpp"

ConfigLocation::ConfigLocation(std::string path): _path(path), _root(DEFAULT_ROOT), _autoindex_status(false), _location_flag(NONE){}

void ConfigLocation::addRoot(std::string root)
{
	this->_root = root;
}

void ConfigLocation::addClientBodySize(size_t client_body_size)
{
	this->_client_body_size = client_body_size;
}

void	ConfigLocation::addAllowedMethods(std::string method)
{
	this->_allowed_methods.push_back(method);
}

void	ConfigLocation::addAutoIndex(int status)
{
	this->_autoindex_status = status;
}

void	ConfigLocation::addIndex(std::string index)
{
	this->_index.push_back(index);
}

void	ConfigLocation::addCgi(std::string extention, std::string path)
{
	std::pair<std::string, std::string> cgi(extention, path);
	this->_cgi.push_back(cgi);
}

void ConfigLocation::addReturn(int code, std::string path)
{
	this->_return = std::make_pair(code, path);
}

void ConfigLocation::addLocationFlag(location_flag flag)
{
	this->_location_flag = flag;
}

void ConfigLocation::addUploadStore(std::string path)
{
	this->_upload_store = path;
}

ConfigLocation::location_pointer ConfigLocation::getLocationBlock()
{
	initLocationBlock();
	return this->_location_block;
}

void ConfigLocation::initLocationBlock()
{
	_location_block = SmartPointer<LocationBlock>(new LocationBlock);
	_location_block->_path = _path;
	_location_block->_root = _root;
	_location_block->_index = _index;
	_location_block->_allowed_methods = _allowed_methods;
	_location_block->_cgi = _cgi;
	_location_block->_autoindex_status = _autoindex_status;
	_location_block->_return = _return;
	_location_block->_location_flag = _location_flag;
	_location_block->_upload_store = _upload_store;
}
