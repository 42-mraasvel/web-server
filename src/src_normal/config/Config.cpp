#include "Config.hpp"
#include "settings.hpp"
#include <fcntl.h>
#include <unistd.h>
#include <string>

Config::Config(std::string const & config_file): _file_name(config_file), _server_amount(0)
{
	this->parser();
	ConfigServer dummy;
	dummy.addPort(8080);
	// dummy.addPort(8081);
	// dummy.addPort(8082);
	// dummy.addPort(8083);
	// ConfigServer dummy2;
	// dummy2.addPort(18000);
	// dummy2.addPort(18001);
	// dummy2.addPort(18002);
	_servers.push_back(dummy);
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

// add comment parser
void Config::tokenizer(std::string const & body)
{
	std::string const & delimiters = "\t ";
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
	
	size_t	start = string.find_first_not_of("\n");
	size_t	semicolon_pos = string.find_first_of(";");
	size_t	newline_pos = string.find_first_of("\n", start);
	if (start == std::string::npos)
	{
		return (1);
	}
	if (semicolon_pos == std::string::npos)
	{
		if (newline_pos == std::string::npos)
		{
			semicolon_pos = string.size();
		}
		else
		{
			semicolon_pos = newline_pos;
		}
	}
	if (newline_pos == std::string::npos)
	{
			newline_pos = string.size();
	}
	std::string config = string.substr(start, semicolon_pos - start);
	std::string comment = string.substr(semicolon_pos, newline_pos - semicolon_pos);
	std::string newline = string.substr(newline_pos, 1);
	if (config.size() > 0)
	{
		_tokens.push_back(config);
	}
	if (comment.size() > 0)
	{
		_tokens.push_back(comment);
	}
	if (newline.size() > 0)
	{
		_tokens.push_back(newline);
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
		return ERR;
	}
	do
	{
		if ((ret = read(fd, buf, BUFFER_SIZE)) == -1)
		{
			return ERR;
		}
		buf[ret] = 0;
		body += buf;

	} while(ret > 0);
	close(fd);
	this->tokenizer(body);
	// for (size_t i = 0; i < _tokens.size(); i++)
	// {
	// 	printf("TOKEN:%lu->%s<-\n", i, _tokens[i].c_str());
	// }
	if (parseConfigFile() == ERR)
	{
		return ERR;
	}
	return (0);
}


// TODO: Add protection for new configserver
int	Config::parseConfigFile()
{
	
	size_t i = 0;
	while (i < _tokens.size())
	{
		if (static_cast<int>(_tokens.size()) - i < 3)
		{
			return ERR;
		}
		if (_tokens[i].compare("server") || _tokens[i + 1].compare("{"))
		{
			return ERR;
		}
		ConfigServer *new_server = new ConfigServer;
		_servers.push_back(*new_server);
		i = parseServer(i);
		_server_amount++;
		if (_tokens[i].compare("}"))
		{
			return ERR;
		}
		i++;
	}
	return (0);
}

int	Config::parseServer(int i)
{
	while (i < static_cast<int>(_tokens.size()) && _tokens[i].compare("}"))
	{
		if (_tokens[i].compare("listen") == 0)
		{
			i = parseListen(i);
		}
		// else if (_tokens[i].compare("client_body_size"))
		// {
			
		// }
		// else if (_tokens[i].compare("allowed_methods"))
		// {

		// }
		// else if (_tokens[i].compare("autoindex"))
		// {
			
		// }
		// else if (_tokens[i].compare("location"))
		// {
			
		// }
		i++;
	}

	return (i);
}

// TODO: add digit protection
int	Config::parseListen(int i)
{
	i++;
	while (_tokens[i].find_first_of(";") == std::string::npos)
	{
		int port = atoi(_tokens[i].c_str());
		_servers[_server_amount].addPort(port);
		i++;
	}
	while (_tokens[i].compare("\n"))
	{
		i++;
	}
	return (i);
}


/* Debugging */

void Config::print() const
{
	std::cout << "Configuration File: " << _file_name << std::endl;
	for (const_iterator it = begin(); it != end(); ++it)
	{
		std::cout << MAGENTA_BOLD "Server" RESET_COLOR " #" << (it - begin() + 1) << std::endl;
		it->print();
	}
}
