#pragma once
#include "fd/FdTable.hpp"
#include "config/Config.hpp"

class Webserver
{
	public:
		int	runWebserver();
		int initWebserver(Config const & config);

	private:
		FdTable _fd_table;
};
