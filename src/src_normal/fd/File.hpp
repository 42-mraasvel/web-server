#pragma once
#include "AFdInfo.hpp"
#include <string>

class Client;

class File : public AFdInfo
{
	public:
		File(int client_index, int fd);
		struct pollfd	getPollFd() const;

		int	readEvent(FdTable & fd_table);
		int	writeEvent(FdTable & fd_table);
		int	closeEvent();

		std::string	const &	getContent() const;
		void				setContent(std::string const & content);

	private:
		std::string	_content;
		int			_client_index;
};
