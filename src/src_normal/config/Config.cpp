#include "Config.hpp"
#include "settings.hpp"
#include "utility/Output.hpp"
#include <fcntl.h>
#include "utility/utility.hpp"
#include <unistd.h>

Config::Config(std::string const & config_file): _file_name(config_file), _server_amount(0), _token_index(0){}

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

void Config::splitToken(std::string str)
{
	size_t pos_operator = str.find_first_of("{;}");
	if (pos_operator == std::string::npos)
	{
		_tokens.push_back(str.substr(0));
		return ;
	}
	if (pos_operator == 0)
	{
		_tokens.push_back(str.substr(0,1));
		if (str.size() > 1)
		{
			splitToken(str.substr(1));
		}
	}
	else
	{
		_tokens.push_back(str.substr(0, pos_operator));
		splitToken(str.substr(pos_operator));
	}
}

int	Config::validateAddressMap()
{
	if (_address_map.empty())
	{
		return configError("address map is empty");
	}
	for (const_iterator_map it = _address_map.begin(); it != _address_map.end(); ++it)
	{
		for (size_t i = 0; i < it->second.size(); i++)
		{
			if (validateServerBlock(*(it->second[i])) == ERR)
			{
				return ERR;
			}
		}
	}
	return OK;
}

int	Config::validateServerBlock(ServerBlock server_block)
{
	if (server_block._locations.size() == 0)
	{
		return configError("server block is missing");
	}
	return OK;
}

int Config::validateToken(std::string token)
{
	std::string arr[] = {
		"listen",
		"server_name",
		"client_body_size",
		"error_page",
		"location",
		"root",
		"index",
		"allowed_methods",
		"autoindex",
		"cgi",
		"upload_store",
		"return",
		"{",
		"}"
	};
	for (size_t i = 0; i < 14; i++)
	{
		if (token.compare(arr[i]) == 0)
		{
			return ERR;
		}
	}
	return OK;
}

int Config::parser()
{
	int	fd;
	int ret;
	char  buf[BUFFER_SIZE + 1];
	std::string body = "";

	if ((fd = open(_file_name.c_str(), O_RDONLY)) == -1)
	{
		return configError("filename: " + _file_name);
	}
	do
	{
		if ((ret = read(fd, buf, BUFFER_SIZE)) == -1)
		{
			return configError("read");
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
	if (initAddressMap() == ERR)
	{
		return ERR;
	}
	if (validateAddressMap() == ERR)
	{
		return ERR;
	}
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
	if (_tokens.size() < 3 || _tokens[_token_index].compare("server") || _tokens[_token_index + 1].compare("{"))
	{
		return configError("invalid configuration");
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
			else if (i == 4)
			{
				return ERR;
			}
			if (_token_index >= _tokens.size())
			{
				return configError("unexpected " + _tokens[_token_index - 1]);
			}
			if (_tokens[_token_index].compare("}") == 0)
			{
				return OK;
			}
		}
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
	if (checkExpectedSyntax("{"))
	{
		return configError("expected '{' instead of " + _tokens[_token_index]);
	}
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
	while (_token_index < _tokens.size() && _tokens[_token_index].compare("}"))
	{
		for (size_t i = 0; i < 7; i++)
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
			else if (i == 6)
			{
				return configError("unexpected " + _tokens[_token_index]);
			}
			if (_tokens[_token_index].compare("}") == 0)
			{
				_token_index++;
				return OK;
			}
		}
	}
	_token_index++;
	return OK;
}

int	Config::parseListen()
{
	_token_index++;
	if (_tokens[_token_index].compare(";") == 0)
	{
		return configError("Listen missing argument");
	}
	size_t split = _tokens[_token_index].find_last_of(":");
	std::string listen;
	std::string host;
	if (split != std::string::npos)
	{
		host = _tokens[_token_index].substr(0, split);
		size_t start = _tokens[_token_index].find_first_not_of(":", split);
		if (start == std::string::npos ||!WebservUtility::validIpv4(host))
		{
			return configError("Listen config error");
		}
		listen = _tokens[_token_index].substr(start);
	}
	else
	{
		host = DEFAULT_ADDRESS;
		listen = _tokens[_token_index].substr(0);
	}
	for (size_t i = 0; i < listen.size(); i++)
	{
		if (std::isdigit(listen[i]) == 0)
		{
			return configError("unexpected syntax: " + listen);
		}
	}
	int port = atoi(listen.c_str());
	_servers[_server_amount].addAddress(host, port);
	_token_index++;
	if(_tokens[_token_index].compare(";") != 0)
	{
		return configError("expected ';' instead of " + _tokens[_token_index]);
	}
	_token_index++;
	return OK;
}

int	Config::parseServerName()
{
	_token_index++;
	if (_tokens[_token_index].compare(";") == 0)
	{
		return configError("server_name missing argument(s)");
	}
	while (_tokens[_token_index].compare(";") && validateToken(_tokens[_token_index]) == OK)
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
		return configError("expected ';' instead of " + _tokens[_token_index]);
	}
	_token_index++;
	return OK;
}

int	Config::parseRoot()
{
	_token_index++;
	if (_tokens[_token_index].compare(";") == 0)
	{
		return configError("root missing argument");
	}
	std::string path;
	if (validateToken(_tokens[_token_index]) == OK)
	{
		path = _tokens[_token_index];
		if (path.find_last_of("/") == path.size() - 1 && path.size() != 1)
		{
			return configError("Root cannot be directory");
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
		return configError("expected ';' instead of " + _tokens[_token_index]);
	}
	_token_index++;
	return OK;
}

int	Config::parseClientBodySize()
{
	_token_index++;
	if (_tokens[_token_index].compare(";") == 0)
	{
		return configError("client_body_size missing argument");
	}
	std::string client_body_size = _tokens[_token_index];
	for (size_t i = 0; i < client_body_size.size(); i++)
	{
		if (std::isdigit(client_body_size[i]) == 0)
		{
			if (i == 0)
			{
				return configError("unexpected syntax: " + client_body_size);
			}
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
				return configError("unexpected syntax: " + client_body_size);
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
		return configError("expected ';' instead of " + _tokens[_token_index]);
	}
	_token_index++;
	return OK;
}

int	Config::parseAllowedMethods()
{
	_token_index++;
	if (_tokens[_token_index].compare(";") == 0)
	{
		return configError("allowed_methods missing argument(s)");
	}
	while (_tokens[_token_index].compare(";") && validateToken(_tokens[_token_index]) == OK)
	{
		if (checkExpectedSyntax("GET", "POST", "DELETE") == ERR)
		{
			return configError("invalid method " + _tokens[_token_index]);
		}
		_servers[_server_amount].addAllowedMethods(_tokens[_token_index]);
		_token_index++;
	}
	if(_tokens[_token_index].compare(";") != 0)
	{
		return configError("expected ';' instead of " + _tokens[_token_index]);
	}
	_token_index++;
	return OK;
}

int	Config::parseAutoindex()
{
	_token_index++;
	if (_tokens[_token_index].compare(";") == 0)
	{
		return configError("autoindex missing argument");
	}
	if (checkExpectedSyntax("on", "off") == ERR)
	{
		return configError("invalid autoindex argument " + _tokens[_token_index]);
	}
	_servers[_server_amount].addAutoIndex(_tokens[_token_index].compare("off"));
	_token_index++;
	if(_tokens[_token_index].compare(";") != 0)
	{
		return configError("expected ';' instead of " + _tokens[_token_index]);
	}
	_token_index++;
	return OK;
}

int	Config::parseErrorPage()
{
	_token_index++;
	if (_tokens[_token_index].compare(";") == 0)
	{
		return configError("error_page missing argument");
	}
	if (validateToken(_tokens[_token_index]) == OK)
	{
		for (size_t i = 0; i < _tokens[_token_index].size(); i++)
		{
			if (std::isdigit(_tokens[_token_index][i]) == 0)
			{
				return configError("expected only digits instead of " + _tokens[_token_index]);
			}
		}
	}
	int page_number = atoi(_tokens[_token_index].c_str());
	_token_index++;
	if (_tokens[_token_index].compare(";") == 0)
	{
		return configError("error_page missing argument");
	}
	if (validateToken(_tokens[_token_index]) == OK)
	{
		if (_tokens[_token_index][0] != '/')
		{
			return configError("error_page must start with '/'");
		}
		_servers[_server_amount].addErrorPage(page_number, _tokens[_token_index]);
		_token_index++;
	}
	if(_tokens[_token_index].compare(";") != 0)
	{
		return configError("expected ';' instead of " + _tokens[_token_index]);
	}
	_token_index++;
	return OK;
}

int Config::parseCgi()
{
	std::string extention;
	std::string path;
	_token_index++;
	if (_tokens[_token_index].compare(";") == 0)
	{
		return configError("cgi missing argument(s)");
	}
	if (validateToken(_tokens[_token_index]) == OK)
	{
		extention = _tokens[_token_index];
		_token_index++;
	}
	if (_tokens[_token_index].compare(";") == 0)
	{
		return configError("cgi missing argument(s)");
	}
	if (validateToken(_tokens[_token_index]) == OK)
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
		return configError("expected ';' instead of " + _tokens[_token_index]);
	}
	_token_index++;
	return OK;
}

int Config::parseUploadStore()
{
	_token_index++;
	if (_tokens[_token_index].compare(";") == 0)
	{
		return configError("upload_store missing argument");
	}
	std::string path;
	if (validateToken(_tokens[_token_index]) == OK)
	{
		path = _tokens[_token_index];
		if (path.find_last_of("/") == path.size() - 1 && path.size() != 1)
		{
			return configError("Root cannot be directory");
		}
		if (path[0] != '/')
		{
			char real_path[4096];
			realpath(path.c_str(),real_path);
			path = real_path;
		}
	}
	_token_index++;
	_servers[_server_amount].addUploadStore(path);
	if(_tokens[_token_index].compare(";") != 0)
	{
		return configError("expected ';' instead of " + _tokens[_token_index]);
	}
	_token_index++;
	return OK;
}

int	Config::parseIndex()
{
	_token_index++;
	if (_tokens[_token_index].compare(";") == 0)
	{
		return configError("index missing argument(s)");
	}
	while (_tokens[_token_index].compare(";") != 0 && validateToken(_tokens[_token_index]) == OK)
	{
		if (_tokens[_token_index].find_last_of("/") == _tokens[_token_index].size() - 1)
		{
			return configError("Index cannot be directory");
		}
		if (_tokens[_token_index].find_last_of(".") == _tokens[_token_index].size() - 1)
		{
			return configError("Index cannot end with a '.'");
		}
		_servers[_server_amount].addIndex(_tokens[_token_index]);
		_token_index++;
	}
	if(_tokens[_token_index].compare(";") != 0)
	{
		return configError("expected ';' instead of " + _tokens[_token_index]);
	}
	_token_index++;
	return OK;
}

int Config::parseReturn()
{
	_token_index++;
	if (_tokens[_token_index].compare(";") == 0)
	{
		return configError("return missing argument(s)");
	}
	std::string ret;
	std::string path;
	if (validateToken(_tokens[_token_index]) == OK)
	{
		ret = _tokens[_token_index];
		for (size_t i = 0; i < ret.size(); i++)
		{
			if (std::isdigit(ret[i]) == 0)
			{
				return configError("return config error");
			}
		}
		_token_index++;
	}
	if (validateToken(_tokens[_token_index]) == OK)
	{
		path = _tokens[_token_index];
		_token_index++;
	}
	int code = WebservUtility::strtoul(ret);
	_servers[_server_amount].addReturn(code, path);
	if(_tokens[_token_index].compare(";") != 0)
	{
		return configError("expected ';' instead of " + _tokens[_token_index]);
	}
	_token_index++;
	return OK;
}

int	Config::checkExpectedSyntax(std::string str)
{
	if (_tokens[_token_index].compare(str) != 0)
	{
		PRINT_ERR << RED_BOLD "Config error: expected " << str << " instead of " << _tokens[_token_index] <<RESET_COLOR << std::endl;
		return ERR;
	}
	return OK;
}

int	Config::checkExpectedSyntax(std::string str1, std::string str2)
{
	if (_tokens[_token_index].compare(str1) != 0 
		&& _tokens[_token_index].compare(str2) != 0)
	{
		PRINT_ERR << RED_BOLD "Config error: expected " << str1 <<" or " << str2 << " instead of " << _tokens[_token_index] <<RESET_COLOR << std::endl;
		return ERR;
	}
	return OK;
}

int	Config::checkExpectedSyntax(std::string str1, std::string str2, std::string str3)
{
	if (_tokens[_token_index].compare(str1) != 0 
		&& _tokens[_token_index].compare(str2) != 0
		&& _tokens[_token_index].compare(str3) != 0)
	{
		PRINT_ERR << RED_BOLD "Config error: expected " << str1 <<" or " << str2 <<" or " << str3 << " instead of " << _tokens[_token_index] <<RESET_COLOR << std::endl;
		return ERR;
	}
	return OK;
}

int	Config::configError(std::string str)
{
	PRINT_ERR << RED_BOLD << "Config error: " << str << std::endl;
	return ERR;
}

int	Config::initAddressMap()
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
	return OK;
}

std::map<std::pair<std::string, int>, std::vector<ConfigServer::server_pointer> >	Config::getAddressMap() const
{
	return this->_address_map;
}
