#include "Config.hpp"
#include "settings.hpp"
#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <cstdlib>
#include <string>
#include "utility/utility.hpp"
#include <map>
#include <limits>

Config::Config(std::string const & config_file): _file_name(config_file), _server_amount(0), _token_index(0)
{}

Config::~Config(){}

Config::const_iterator Config::begin() const
{
	return (this->_servers.begin());
}

Config::const_iterator Config::end() const
{
	return (this->_servers.end());
}

void Config::tokenizer(std::string const & body)
{
	std::string const & delimiters = "\n\t ";
	size_t pos, start = 0;
	while(true)
	{	
		pos = body.find_first_of(delimiters, start);
		if (pos == std::string::npos)
		{
			break;
		}
		if (pos - start > 0)
		{
			splitToken(body.substr(start, pos - start));
		}
		start = body.find_first_not_of(delimiters, pos);
	}
	if (pos - start > 0)
	{
		splitToken(body.substr(start, pos - start));
	}
}

void	Config::splitToken(std::string const & string)
{
	size_t pos_open_bracket = string.find_first_of("{");
	size_t pos_close_bracket = string.find_first_of("}");
	size_t pos_semicolon = string.find_first_of(";");
	size_t index = 0;
	if (pos_open_bracket != std::string::npos)
	{
		if (pos_open_bracket > 0)
		{
			_tokens.push_back(string.substr(index, pos_open_bracket - index));
		}
		_tokens.push_back(string.substr(pos_open_bracket, 1));
		splitToken(string.substr(pos_open_bracket + 1));
	}
	else if (pos_semicolon != std::string::npos)
	{
		if (pos_semicolon > 0)
		{
			_tokens.push_back(string.substr(index, pos_semicolon - index));
		}
		_tokens.push_back(string.substr(pos_semicolon, 1));
		splitToken(string.substr(pos_semicolon + 1));
	}
	else if (pos_close_bracket != std::string::npos)
	{
		if (pos_close_bracket)
		{
			_tokens.push_back(string.substr(index, pos_close_bracket - index));
		}
		_tokens.push_back(string.substr(pos_close_bracket, 1));
		splitToken(string.substr(pos_close_bracket + 1));
	}
	else if (string.size() > 0)
	{
		_tokens.push_back(string.substr(0));
	}
}

int Config::parser()
{
	int	fd;
	int ret;
	char  buf[BUFFER_SIZE + 1];
	std::string body = "";

	if ((fd = open(_file_name.c_str(), O_RDONLY)) == -1)
	{
		configError("filename: " + _file_name);
		return ERR;
	}
	do
	{
		if ((ret = read(fd, buf, BUFFER_SIZE)) == -1)
		{
			configError("read");
			return ERR;
		}
		buf[ret] = 0;
		body += buf;

	} while(ret > 0);
	close(fd);
	tokenizer(body);
	if (parseConfigFile() == ERR)
	{
		return ERR;
	}
	initAddressMap();
	printAddressMap();
	return OK;
}


int	Config::parseConfigFile()
{
	if (_tokens.size() < 3)
	{
		return ERR;
	}
	while (_token_index < _tokens.size())
	{
		if (parseServer() == ERR)
		{
			return ERR;
		}
		_server_amount++;
		_token_index++;
	}
	return OK;
}


int Config::parseServer()
{
	_servers.push_back(ConfigServer());
	int ret = 0;
	if (_tokens.size() < 3
		|| _tokens[_token_index].compare("server")
		|| _tokens[_token_index + 1].compare("{"))
	{
		return ERR;
	}
	_token_index+=2;
	static parseFunctions func[] = 
	{
		{"listen", &Config::parseListen},
		{"server_name", &Config::parseServerName},
		{"error_page", &Config::parseErrorPage},
		{"client_body_size", &Config::parseClientBodySize},
		{"location", &Config::parseLocation}
	};
	while (_token_index < _tokens.size() && _tokens[_token_index].compare("}"))
	{
		for (size_t i = 0; i < 5; i++)
		{
			ret = 0;
			if (_tokens[_token_index].compare(func[i].str) == 0)
			{
				ret = (this->*(func[i].f))();
				if (ret == ERR)
				{
					return ERR;
				}
				i = -1;
			}
			else if(i == 4 && _tokens[_token_index].compare("}"))
			{
				return ERR;
			}
		}
		_token_index++;
	}
	if (_servers[_server_amount].emptyAddress() == 0)
	{
		_servers[_server_amount].addAddress("0.0.0.0", 80);
	}
	if (_tokens[_token_index].compare("}"))
	{
		return ERR;
	}
	return OK;

}



int Config::parseLocation()
{
	location_flag flag = NONE;
	int ret = 0;
	_token_index++;
	if (_tokens[_token_index].compare("=") == 0)
	{
		flag = EQUAL;
		_token_index++;
	}
	_servers[_server_amount].addLocation(ConfigLocation(_tokens[_token_index]));
	_servers[_server_amount].addLocationFlag(flag);
	_token_index++;
	checkExpectedSyntax("{");
	_token_index++;
	static parseFunctions func[] = 
	{
		{"root", &Config::parseRoot},
		{"allowed_methods", &Config::parseAllowedMethods},
		{"autoindex", &Config::parseAutoindex},
		{"index", &Config::parseIndex},
		{"cgi", &Config::parseCgi},
		{"return", &Config::parseReturn},
		{"upload_store", &Config::parseUploadStore}
	};
	bool root_check = false;
	while (_token_index < _tokens.size() && _tokens[_token_index].compare("}"))
	{
		for (size_t i = 0; i < 8; i++)
		{
			ret = 0;
			if (_tokens[_token_index].compare(func[i].str) == 0)
			{
				if (_tokens[_token_index].compare("root") == 0)
				{
					root_check = true;
				}
				ret = (this->*(func[i].f))();
				if (ret == ERR)
				{
					return ERR;
				}
				i = -1;
			}
			else if(i ==7  && _tokens[_token_index].compare("}") == 0)
			{
				return OK;
			}
		}
		_token_index++;
	}
	if (root_check == false)
	{
		_servers[_server_amount].addRoot("/var/www");
	}
	if (_tokens[_token_index].compare("}"))
	{
		return ERR;
	}
	return OK;
}

// TODO: add protection
int	Config::parseListen()
{
	_token_index++;
	size_t split = _tokens[_token_index].find_last_of(":");
	std::string listen;
	std::string host;
	if (split != std::string::npos)
	{
		host = _tokens[_token_index].substr(0, split);
		size_t start = _tokens[_token_index].find_first_not_of(":", split);
		if (start == std::string::npos)
		{
			return ERR;
			// configError("Listen config error");
		}
		listen = _tokens[_token_index].substr(start);
	}
	else
	{
		host = "0.0.0.0";
		listen = _tokens[_token_index].substr(0);
	}
	for (size_t i = 0; i < listen.size(); i++)
	{
		if (std::isdigit(listen[i]) == 0)
		{
			return ERR;
			// configError("unexpected syntax: " + listen);
		}
	}
	int port = atoi(listen.c_str());
	_servers[_server_amount].addAddress(host, port);
	_token_index++;
	if(_tokens[_token_index].compare(";") != 0)
	{
		return (ERR);
	}
	_token_index++;
	return OK;
}

int	Config::parseServerName()
{
	_token_index++;
	while (_tokens[_token_index].compare(";") != 0)
	{
		if (_tokens[_token_index].compare("\"\"") == 0)
		{
			_servers[_server_amount].addServerName("");
		}
		else
		{
			_servers[_server_amount].addServerName(_tokens[_token_index]);
		}
		_token_index++;
	}
	if(_tokens[_token_index].compare(";") != 0)
	{
		return (ERR);
	}
	_token_index++;
	return (OK);
}

int	Config::parseRoot()
{
	_token_index++;
	std::string path;
	if (_tokens[_token_index].compare(";"))
	{
		path = _tokens[_token_index];
		if (path.find_last_of("/") == path.size() - 1)
		{
			configError("Root cannot be directory");
		}
		if (path[0] != '/')
		{
			char real_path[4096];
			realpath(path.c_str(),real_path);
			path = real_path;
		}
		_servers[_server_amount].addRoot(path);
	}
	_token_index++;
	if(_tokens[_token_index].compare(";") != 0)
	{
		return (ERR);
	}
	_token_index++;
	return (OK);
}

int	Config::parseClientBodySize()
{
	_token_index++;
	std::string client_body_size = _tokens[_token_index];
	for (size_t i = 0; i < client_body_size.size(); i++)
	{

		if (std::isdigit(client_body_size[i]) == 0)
		{

			client_body_size[i] =toupper(client_body_size[i]);

			if (client_body_size.find_first_of("KGM") == client_body_size.size() - 1)
			{
				if (client_body_size[i] == 'K')
				{
					client_body_size = client_body_size.substr(0, i).append("000");
				}
				else if(client_body_size[i]== 'M')
				{
					client_body_size = client_body_size.substr(0, i).append("000000");
				}
				else if(client_body_size[i] == 'G')
				{
					client_body_size = client_body_size.substr(0, i).append("000000000");
				}
			}
			else
			{
				configError("unexpected syntax: " + client_body_size);
			}
		}
	}
	std::size_t size = WebservUtility::strtoul(client_body_size);
	if (size == 0)
	{
		size = std::numeric_limits<std::size_t>::max();
	}
	_servers[_server_amount].addClientBodySize(size);
	_token_index++;
	if(_tokens[_token_index].compare(";") != 0)
	{
		return (ERR);
	}
	_token_index++;
	return (OK);
}

int	Config::parseAllowedMethods()
{
	_token_index++;
	while (_tokens[_token_index].compare(";") != 0)
	{
		if (checkExpectedSyntax("GET", "POST", "DELETE") == OK)
		{
			_servers[_server_amount].addAllowedMethods(_tokens[_token_index]);
		}
		_token_index++;
	}
	if(_tokens[_token_index].compare(";") != 0)
	{
		return (ERR);
	}
	_token_index++;
	return (OK);
}

int	Config::parseAutoindex()
{
	_token_index++;
	if (checkExpectedSyntax("on", "off") == OK)
	{
		_servers[_server_amount].addAutoIndex(_tokens[_token_index].compare("off"));
	}
	_token_index++;
	if(_tokens[_token_index].compare(";") != 0)
	{
		return (ERR);
	}
	_token_index++;
	return (OK);
}

int	Config::parseErrorPage()
{
	_token_index++;
	for (size_t i = 0; i < _tokens[_token_index].size(); i++)
	{
		if (std::isdigit(_tokens[_token_index][i]) == 0)
		{
			configError(_tokens[_token_index]);
		}
	}
	int page_number = atoi(_tokens[_token_index].c_str());
	_token_index++;
	_servers[_server_amount].addErrorPage(page_number, _tokens[_token_index]);
	_token_index++;
	if(_tokens[_token_index].compare(";") != 0)
	{
		return (ERR);
	}
	_token_index++;
	return (OK);
}

int Config::parseCgi()
{
	_token_index++;
	std::string extention;
	std::string path;
	if (_tokens[_token_index].compare(";") != 0)
	{
		extention = _tokens[_token_index];
		_token_index++;
	}
	if (_tokens[_token_index].compare(";") != 0)
	{
		path = _tokens[_token_index];
		_token_index++;
	}
	if (path[0] != '/')
	{
		char real_path[4096];
		realpath(path.c_str(),real_path);
		path = real_path;
	}
	_servers[_server_amount].addCgi(extention, path);
	if(_tokens[_token_index].compare(";") != 0)
	{
		return (ERR);
	}
	_token_index++;
	return (OK);
}


int Config::parseUploadStore()
{
	_token_index++;
	std::string path;
	if (_tokens[_token_index].compare(";") != 0)
	{
		path = _tokens[_token_index];
		_token_index++;
	}
	if (path[0] != '/')
	{
		char real_path[4096];
		realpath(path.c_str(),real_path);
		path = real_path;
	}
	_servers[_server_amount].addUploadStore(path);
	if(_tokens[_token_index].compare(";") != 0)
	{
		return (ERR);
	}
	_token_index++;
	return (OK);
}

int	Config::parseIndex()
{
	_token_index++;
	// while (_tokens[_token_index].compare(";") != 0 && _tokens[_token_index].find_first_of(".") != std::string::npos)
	while (_tokens[_token_index].compare(";"))
	{
		if (_tokens[_token_index].find_last_of("/") == _tokens[_token_index].size() - 1)
		{
			configError("Index cannot be directory");
		}
		_servers[_server_amount].addIndex(_tokens[_token_index]);
		_token_index++;
	}
	if(_tokens[_token_index].compare(";") != 0)
	{
		return (ERR);
	}
	_token_index++;
	return (OK);
}

int Config::parseReturn()
{
	_token_index++;
	std::string ret;
	std::string path;
	if (_tokens[_token_index].compare(";") != 0)
	{
		ret = _tokens[_token_index];
		_token_index++;
	}
	if (_tokens[_token_index].compare(";") != 0)
	{
		path = _tokens[_token_index];
		_token_index++;
	}
	int code = WebservUtility::strtoul(ret);
	_servers[_server_amount].addReturn(code, path);
	if(_tokens[_token_index].compare(";") != 0)
	{
		return (ERR);
	}
	_token_index++;
	return (OK);
}

int	Config::checkExpectedSyntax(std::string str)
{
	if (_tokens[_token_index].compare(str) != 0)
	{
		// abortProgram("Config Error: expected " + str + " instead of " + _tokens[_token_index]);
		return ERR;
	}
	return (OK);
}

int	Config::checkExpectedSyntax(std::string str1, std::string str2)
{
	if (_tokens[_token_index].compare(str1) != 0 
		&& _tokens[_token_index].compare(str2) != 0)
	{
		abortProgram("Config Error: expected " + str1 + " or " + str2 + " instead of " + _tokens[_token_index]);
	}
	return (OK);
}

int	Config::checkExpectedSyntax(std::string str1, std::string str2, std::string str3)
{
	if (_tokens[_token_index].compare(str1) != 0 
		&& _tokens[_token_index].compare(str2) != 0
		&& _tokens[_token_index].compare(str3) != 0)
	{
		PRINT_ERR << RED_BOLD "Config Error: expected " << str1 <<" or " << str2 <<" or " << str3 << " instead of " << _tokens[_token_index] <<RESET_COLOR << std::endl;
		return ERR;
	}
	return (OK);
}

void	Config::configError(std::string str)
{
	PRINT_ERR << RED_BOLD << "Config error: " << str << std::endl;
}

// Getters



// Utility
void	Config::initAddressMap()
{
	std::pair<std::map<ip_host_pair,server_block_vector>::iterator,bool> ret;
	std::map<ip_host_pair,server_block_vector>::iterator map_it;
	address_map tmp;

	for (size_t i = 0; i < _servers.size(); i++)
	{
		tmp = _servers[i].getAddressMap();
		for (const_iterator_map it = tmp.begin(); it != tmp.end(); ++it)
		{
			ret = _address_map.insert(std::make_pair(it->first, it->second));
			if (ret.second == false)
			{
				map_it = _address_map.find(it->first);
				map_it->second.push_back(it->second[0]);
			}
		}
	}
}

std::map<std::pair<std::string, int>, std::vector<ConfigServer::server_pointer> >	Config::getAddressMap() const
{
	return this->_address_map;
}


