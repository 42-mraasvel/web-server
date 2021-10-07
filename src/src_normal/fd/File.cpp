#include "File.hpp"
#include "settings.hpp"

File::File(int client_index, int fd): AFdInfo(fd), _client_index(client_index) {}

int File::readEvent(FdTable & fd_table)
{
	return OK;
}

int File::writeEvent(FdTable & fd_table)
{
	return OK;
}
