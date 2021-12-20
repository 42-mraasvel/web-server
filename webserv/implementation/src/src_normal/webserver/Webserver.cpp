#include "Webserver.hpp"
#include "settings.hpp"
#include "fd/Server.hpp"
#include "color.hpp"
#include <sys/socket.h>
#include <poll.h>
#include <unistd.h>
#include <cstdlib> // REMOVE, RM

Webserver::Webserver(address_map map): _config_map(map)
{}

int Webserver::initServer(ip_host_pair iphost)
{
	SmartPointer<Server> new_server(new Server());
	if (new_server->setupServer(iphost, &_config_map) == ERR)
	{
		return ERR;
	}
	_fd_table.insertFd(SmartPointer<AFdInfo>(new_server));
	return OK;
}

int	Webserver::init()
{
	for (Config::const_iterator_map it = _config_map.begin(); it != _config_map.end(); ++it)
	{
		if (shouldInitialize(it->first))
		{
			PRINT_INFO << YELLOW_BOLD "Initializing listening socket" RESET_COLOR ": " \
				<< it->first.first << ":" << it->first.second << std::endl;
			if (initServer(it->first) == ERR)
			{
				return ERR;
			}
		}
	}
	return OK;
}

bool Webserver::shouldInitialize(ip_host_pair const & iphost) const
{
	if (iphost.first != "0.0.0.0")
	{
		if (_config_map.count(ip_host_pair("0.0.0.0", iphost.second)) != 0)
		{
			return false;
		}
	}
	return true;
}

bool Webserver::shouldExecuteFd(const FdTable::AFdPointer afd)
{
	return afd->getFlag() != AFdInfo::TO_ERASE;
}

bool Webserver::shouldCloseFd(short revents) const
{
	return (revents & (POLLERR | POLLNVAL)) ||
		((revents & POLLHUP) && !(revents & POLLIN));
}

void Webserver::executeFd(short revents, FdTable::AFdPointer afd)
{
	if (shouldCloseFd(revents))
	{
		PRINT_DEBUG << BLUE_BOLD "Close Event" RESET_COLOR ": " \
			<< afd->getName() << ": [" << afd->getFd() << "]" << std::endl;
		afd->closeEvent(_fd_table);
		return;
	}

	if (revents & POLLIN)
	{
		PRINT_DEBUG << BLUE_BOLD "Read Event" RESET_COLOR ": " \
			<< afd->getName() << ": [" << afd->getFd() << "]" << std::endl;
		afd->readEvent(_fd_table);
	}

	if (revents & POLLOUT && !(revents & (POLLERR | POLLNVAL | POLLHUP)))
	{
		PRINT_DEBUG << BLUE_BOLD "Write Event" RESET_COLOR ": " \
			<< afd->getName() << ": [" << afd->getFd() << "]" << std::endl;
		afd->writeEvent(_fd_table);
	}
}

int	Webserver::dispatchFd(int ready)
{
	std::size_t i = 0;
	for (std::size_t i = 0; i < _fd_table.size(); ++i)
	{
		if (shouldExecuteFd(_fd_table[i].second))
		{
			try
			{
				executeFd(_fd_table[i].first.revents, _fd_table[i].second);
			}
			catch (std::exception const & e)
			{
				PRINT_ERR << "Exception: [" << e.what() << "]" << std::endl;
				_fd_table[i].second->exceptionEvent(_fd_table);
			}
		}
	}
	return OK;
}

void	Webserver::scanFdTable()
{
	for (std::size_t i = 0; i < _fd_table.size(); ++i)
	{
		if (_fd_table[i].second->getFlag() == AFdInfo::TO_ERASE)
		{
			continue;
		}
		try
		{
			_fd_table[i].second->update(_fd_table);
		}
		catch (std::exception const & e)
		{
			PRINT_ERR << "Update Exception: " << e.what() << std::endl;
			_fd_table[i].second->exceptionEvent(_fd_table);
		}
	}

	std::size_t i = 0;
	while (i < _fd_table.size())
	{
		if (_fd_table[i].second->getFlag() == AFdInfo::TO_ERASE)
		{
			PRINT_DEBUG << "Erasing Fd: " << _fd_table[i].second->getName() \
				<< ": [" << _fd_table[i].second->getFd() << "]" << std::endl;
			_fd_table.eraseFd(i);
			continue;
		}
		++i;
	}
}

/*
Poll loop!
*/
int	Webserver::run()
{
	int ready;

	printOpening();
	while(true)
	{
		scanFdTable();
		ready = poll(_fd_table.getPointer(), _fd_table.size(), POLL_TIMEOUT);
		PRINT_DEBUG << "Number of Fds: " << _fd_table.size() << std::endl;
		if (ready < 0)
		{
			syscallError(_FUNC_ERR("poll"));
			// TODO: evaluate poll error
		}
		else if (ready > 0)
		{
			print();
			dispatchFd(ready);
		}
	}

	return OK;
}

void Webserver::printOpening() const
{
	PRINT << MAGENTA_BOLD << "-- Listening --" << RESET_COLOR << std::endl;
}

/* Debugging */

void Webserver::print() const
{
	_fd_table.print();
}
