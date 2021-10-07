#include "File.hpp"
#include "settings.hpp"

int	File::setClientIndex(std::size_t  client_index)
{
	_client_index = client_index;
}


int File::readEvent()
{
	return OK;
}

int File::writeEvent()
{
	return OK;
}
