#include "CgiReader.hpp"
#include "settings.hpp"
#include <poll.h>
#include <cassert>

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
    return OK;
}
