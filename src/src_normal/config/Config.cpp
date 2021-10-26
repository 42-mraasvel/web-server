#include "Config.hpp"
#include "settings.hpp"
#include <fcntl.h>
#include <unistd.h>
#include <cstdlib>
#include <string>

Config::Config(std::string const & config_file): _file_name(config_file), _server_amount(0), _token_index(0)
{
	this->parser();
	// ConfigServer dummy;
	// dummy.addPort(8080);
	// dummy.addPort(8081);
	// dummy.addPort(8082);
	// dummy.addPort(8083);
	// ConfigServer dummy2;
	// dummy2.addPort(18000);
	// dummy2.addPort(18001);
	// dummy2.addPort(18002);
	// _servers.push_back(dummy);
	// _servers.push_back(dummy2);
}

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

int	Config::splitToken(std::string const & string)
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
	return (0);
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
	}
	do
	{
		if ((ret = read(fd, buf, BUFFER_SIZE)) == -1)
		{
			configError("read");
		}
		buf[ret] = 0;
		body += buf;

	} while(ret > 0);
	close(fd);
	this->tokenizer(body);
	parseConfigFile();
	return (0);
}


// TODO: Add protection for new configserver
int	Config::parseConfigFile()
{
	while (_token_index < _tokens.size())
	{
		parseServer();
		_server_amount++;
		_token_index++;
	}
	return (OK);
}

int	Config::parseServer()
{
	_servers.push_back(ConfigServer());
	if (_tokens.size() < 3)
	{
		configError("Configuration Error");
	}
	checkExpectedSyntax("server");
	_token_index++;
	checkExpectedSyntax("{");
	_token_index++;
	while (_token_index < _tokens.size() && _tokens[_token_index].compare("}"))
	{
		if (_tokens[_token_index].compare("listen") == 0)
		{
			parseListen();
		}
		else if (_tokens[_token_index].compare("server_name") == 0)
		{
			parseServerName();
		}
		else if (_tokens[_token_index].compare("error_page") == 0)
		{
			parseErrorPage();
		}
		else if (_tokens[_token_index].compare("location") == 0)
		{
			parseLocation();
			_token_index++;
			continue;
		}
		else
		{
			std::cout << RED_BOLD "Config Error: '" << _tokens[_token_index] << "' is not a valid configuration" RESET_COLOR << std::endl;
			exit(1);
		}
		checkExpectedSyntax(";");
		_token_index++;
	}
	if (_servers[_server_amount].portIsEmpty())
	{
		_servers[_server_amount].addPort(80);
	}
	if (_servers[_server_amount].hostIsEmpty())
	{
		_servers[_server_amount].addHostName("localhost");
		_servers[_server_amount].addServerName("localhost");
	}
	checkExpectedSyntax("}");
	return (OK);
}

int	Config::parseLocation()
{
	_token_index++;
	_servers[_server_amount].addLocation(ConfigLocation(_tokens[_token_index]));
	_token_index++;
	checkExpectedSyntax("{");
	_token_index++;
	while(_token_index < _tokens.size() && _tokens[_token_index].compare("}"))
	{
		if (_tokens[_token_index].compare("root") == 0)
		{
			parseRoot();
		}
		else if (_tokens[_token_index].compare("client_body_size") == 0)
		{
			parseClientBodySize();
		}
		else if (_tokens[_token_index].compare("allowed_methods") == 0)
		{
			parseAllowedMethods();
		}
		else if (_tokens[_token_index].compare("autoindex") == 0)
		{
			parseAutoindex();
		}
		else if (_tokens[_token_index].compare("index") == 0)
		{
			parseIndex();
		}
		checkExpectedSyntax(";");
		_token_index++;
	}
	return (1);
}

// TODO: add protection
int	Config::parseListen()
{
	_token_index++;
	size_t split = _tokens[_token_index].find_last_of(":");
	std::string listen;
	if (split != std::string::npos)
	{
		std::string host = _tokens[_token_index].substr(0, split);
		if (host.find_first_of(":") == std::string::npos && _servers[_server_amount].hostIsEmpty())
		{
			_servers[_server_amount].addHostName(host);
		}
		size_t start = _tokens[_token_index].find_first_not_of(":", split);
		if (start == std::string::npos)
		{
			configError("Listen config error");
		}
		listen = _tokens[_token_index].substr(start);

	}
	else
	{
		listen = _tokens[_token_index].substr(0);
	}
	for (size_t i = 0; i < listen.size(); i++)
	{
		if (std::isdigit(listen[i]) == 0)
		{
			configError("unexpected syntax: " + listen);
		}
	}
	int port = atoi(listen.c_str());
	_servers[_server_amount].addPort(port);
	_token_index++;
	return (_token_index);
}

int	Config::parseServerName()
{
	_token_index++;
	while (_tokens[_token_index].compare(";") != 0)
	{
		if (_servers[_server_amount].hostIsEmpty())
		{
			_servers[_server_amount].addHostName(_tokens[_token_index]);
			_servers[_server_amount].addServerName(_tokens[_token_index]);
		}
		else if (checkServerName(_tokens[_token_index]))
		{
			_servers[_server_amount].addServerName(_tokens[_token_index]);
		}
		_token_index++;
	}
	return (_token_index);
}

int	Config::checkServerName(std::string name)
{
	std::string www = "www.";
	std::string nl = ".nl";
	std::string com = ".com";
	std::string host = _servers[_server_amount].getHostName();
	std::string variant_1 = www.substr(0).append(host);
	std::string variant_2 = www.substr(0).append(host).append(com);
	std::string variant_3 = www.substr(0).append(host).append(nl);
	std::string variant_4 = host.substr(0).append(com);
	std::string variant_5 = host.substr(0).append(nl);
	if (variant_1.compare(name)
		&& variant_2.compare(name)
		&& variant_3.compare(name)
		&& variant_4.compare(name)
		&& variant_5.compare(name)
		&& host.compare(name))
	{
		std::cout << RED_BOLD "Config Error: unexpected servername: '" << name << "' where hostname is '" << host << "'" << RESET_COLOR << std::endl;
		exit(1);
	}
	return (1);
}

int	Config::parseRoot()
{
	_token_index++;
	if (_tokens[_token_index].compare(";"))
	{
		_servers[_server_amount].addRoot(_tokens[_token_index]);
	}
	_token_index++;
	return (1);
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
	size_t size = atol(client_body_size.c_str());
	_servers[_server_amount].addClientBodySize(size);
	_token_index++;
	return (_token_index);
}

int	Config::parseAllowedMethods()
{
	_token_index++;
	while (_tokens[_token_index].compare(";") != 0)
	{
		if (checkExpectedSyntax("GET", "POST", "DELETE"))
		{
			_servers[_server_amount].addAllowedMethods(_tokens[_token_index]);
		}
		_token_index++;
	}
	return (_token_index);
}

int	Config::parseAutoindex()
{
	_token_index++;
	if (checkExpectedSyntax("on", "off"))
	{
		_servers[_server_amount].addAutoIndex(_tokens[_token_index].compare("off"));
	}
	_token_index++;
	return (_token_index);
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
	return (_token_index);
}

int	Config::parseIndex()
{
	_token_index++;
	while (_tokens[_token_index].compare(";") != 0)
	{
		_servers[_server_amount].addIndex(_tokens[_token_index]);
		_token_index++;
	}
	return (_token_index);
}

int	Config::checkExpectedSyntax(std::string str)
{
	if (_tokens[_token_index].compare(str) != 0)
	{
		std::cout << RED_BOLD "Config Error: expected " << str << " instead of " << _tokens[_token_index] << RESET_COLOR << std::endl;
		exit(1);
	}
	return (1);
}

int	Config::checkExpectedSyntax(std::string str1, std::string str2)
{
	if (_tokens[_token_index].compare(str1) != 0 
		&& _tokens[_token_index].compare(str2) != 0)
	{
		std::cout << RED_BOLD "Config Error: expected " << str1 <<" or " << str2 << " instead of " << _tokens[_token_index] <<RESET_COLOR << std::endl;
		exit(1);
	}
	return (1);
}

int	Config::checkExpectedSyntax(std::string str1, std::string str2, std::string str3)
{
	if (_tokens[_token_index].compare(str1) != 0 
		&& _tokens[_token_index].compare(str2) != 0
		&& _tokens[_token_index].compare(str3) != 0)
	{
		std::cout << RED_BOLD "Config Error: expected " << str1 <<" or " << str2 <<" or " << str3 << " instead of " << _tokens[_token_index] <<RESET_COLOR << std::endl;
		exit(1);
	}
	return (1);
}

void	Config::configError(std::string str)
{
	std::cout << RED_BOLD << "Config error: " << str << std::endl;
	exit(1);
}

/* Debugging */
void Config::print() const
{
	for (const_iterator it = begin(); it != end(); ++it)
	{
		std::cout << MAGENTA_BOLD "Server" RESET_COLOR " #" << (it - begin() + 1) << std::endl;
		it->print();
	}
}
