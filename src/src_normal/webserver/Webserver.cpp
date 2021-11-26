#include "Webserver.hpp"
#include "settings.hpp"
#include "fd/Server.hpp"
#include "color.hpp"
#include <sys/socket.h>
#include <poll.h>
#include <unistd.h>
#include <cstdlib> // REMOVE, RM

Webserver::Webserver(Config::address_map map): _config_map(map)
{}

int Webserver::initServer(std::pair<std::string, int> ip_host_pair)
{
	//TODO:fix
	Server *new_server = new Server();
	if (new_server->setupServer(ip_host_pair.second, &_config_map) == ERR)
	{
		delete new_server;
		return ERR;
	}
	_fd_table.insertFd(new_server);
	return OK;
}

/*
TODO: close FD after failure
*/
int	Webserver::init(Config const & config)
{
	printf("Hardcoding: 8080\n");
	SmartPointer<Server> new_server(new Server());
	if (new_server->setupServer(8080, &_config_map) == ERR)
	{
		return ERR;
	}
	_fd_table.insertFd(SmartPointer<AFdInfo>(new_server));
	return OK;
}

bool Webserver::shouldExecuteFd(const FdTable::AFdPointer afd)
{
	return afd->getFlag() != AFdInfo::TO_ERASE;
}

bool Webserver::shouldCloseFd(const struct pollfd & pfd)
{
	//TODO: test on mac if this is how it functions as well
	return (pfd.revents & (POLLERR | POLLNVAL)) ||
		((pfd.revents & POLLHUP) && !(pfd.revents & POLLIN));
}

void Webserver::executeFd(const struct pollfd& pfd, FdTable::AFdPointer afd)
{
	if (shouldCloseFd(pfd))
	{
		printf(BLUE_BOLD "Close Event:" RESET_COLOR " %s: [%d]\n",
			afd->getName().c_str(), pfd.fd);
		afd->closeEvent(_fd_table);
		return;
	}

	if (pfd.revents & POLLIN)
	{
		printf(BLUE_BOLD "Read event:" RESET_COLOR " %s: [%d]\n",
			afd->getName().c_str(), pfd.fd);
		afd->readEvent(_fd_table);
	}

	if (pfd.revents & POLLOUT)
	{
		printf(BLUE_BOLD "Write event:" RESET_COLOR " %s: [%d]\n",
			afd->getName().c_str(), pfd.fd);
		afd->writeEvent(_fd_table);
	}

}

//TODO: evaluate 'ready'
//TODO: add Multithreading
int	Webserver::dispatchFd(int ready)
{
	std::size_t i = 0;
	for (std::size_t i = 0; i < _fd_table.size(); ++i)
	{
		if (shouldExecuteFd(_fd_table[i].second))
		{
			printf("EXECUTING: %lu, %s\n", i, _fd_table[i].second->getName().c_str());
			try {
				executeFd(_fd_table[i].first, _fd_table[i].second);
			} catch (std::exception const & e) {
				fprintf(stderr, "%sEXCEPTION%s: [%s]\n",
					RED_BOLD, RESET_COLOR, e.what());
				_fd_table[i].second->exceptionEvent(_fd_table);
			}
		}
	}
	return OK;
}

//TODO: scan for Timeout
void	Webserver::scanFdTable()
{
	for (std::size_t i = 0; i < _fd_table.size(); ++i)
	{
		try {
			_fd_table[i].second->update(_fd_table);
		} catch (std::exception const & e) {
			fprintf(stderr, "%sUPDATE EXCEPTION%s: [%s]\n",
				RED_BOLD, RESET_COLOR, e.what());
			_fd_table[i].second->exceptionEvent(_fd_table);
		}
	}

	std::size_t i = 0;
	while (i < _fd_table.size())
	{
		if (_fd_table[i].second->getFlag() == AFdInfo::TO_ERASE)
		{
			printf("Erasing Fd: %s: [%d]\n",
				_fd_table[i].second->getName().c_str(), _fd_table[i].second->getFd());
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

	while(true)
	{
		scanFdTable();
		ready = poll(_fd_table.getPointer(), _fd_table.size(), POLL_TIMEOUT);
		printf("Number of connections: %lu\n", _fd_table.size());
		if (ready < 0)
		{
			perror("Poll");
			// TODO possible exit
		}
		else if (ready > 0)
		{
			printf(YELLOW_BOLD "Poll returns: " RESET_COLOR "%d\n", ready);
			print();
			dispatchFd(ready);
		}
		else
		{
			#ifdef __APPLE__
			#ifdef LEAK_CHECK
			//RM, REMOVE
			system("leaks debug.out");
			#endif /* LEAK_CHECK */
			#endif /* __APPLE__ */
		}
	}

	return OK;
}

/* Debugging */

void Webserver::print() const
{
	_fd_table.print();
}
