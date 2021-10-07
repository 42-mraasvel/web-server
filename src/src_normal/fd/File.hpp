#pragma once
#include "AFdInfo.hpp"

class File
{
	public:
		int	readEvent();
		int	writeEvent();

	private:
		std::size_t client_index;
		// std::string file_content;
};
