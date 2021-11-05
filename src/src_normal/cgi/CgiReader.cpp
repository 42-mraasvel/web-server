#include "CgiReader.hpp"
#include "settings.hpp"
#include "utility/macros.hpp"
#include "utility/utility.hpp"
#include <poll.h>
#include <cassert>
#include <unistd.h>

CgiReader::CgiReader(int fd)
: AFdInfo(fd) {}

CgiReader::~CgiReader() {}

struct pollfd CgiReader::getPollFd() const
{
	struct pollfd pfd;

	pfd.fd = _fd;
	pfd.events = POLLIN;
	pfd.revents = 0;
	return pfd;
}

int	CgiReader::writeEvent(FdTable & fd_table)
{
	std::cerr << "Cgi Reader's write event called" << std::endl;
	assert(false);
	return OK;
}

int	CgiReader::readEvent(FdTable & fd_table)
{
	std::string buffer(BUFFER_SIZE + 1, '\0');

	ssize_t n = read(_fd, &buffer[0], BUFFER_SIZE);
	if (n == ERR)
	{
		syscallError(_FUNC_ERR("read"));
		return ERR;
	}

	buffer.resize(n);
	printf("CgiReader: Read [%ld] bytes\n", n);
	std::cout << buffer << std::endl;
	return OK;
}
