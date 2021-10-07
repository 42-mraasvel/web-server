#pragma once
#include "AFdInfo.hpp"

class File : public AFdInfo
{
	public:
		File(int client_index, int fd);

		int	readEvent(FdTable & fd_table);
		int	writeEvent(FdTable & fd_table);


	private:
		std::size_t _client_index;
		// std::string file_content;
};
