#pragma once
#include "AFdInfo.hpp"

class File : public AFdInfo
{
	public:
		int	setClientIndex(std::size_t  client_index);

		int	readEvent();
		int	writeEvent();


	private:
		std::size_t _client_index;
		// std::string file_content;
};
