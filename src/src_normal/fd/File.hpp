#pragma once
#include "AFdInfo.hpp"
#include <string>

class Client;

class File : public AFdInfo
{
	public:
		File(int fd);
		struct pollfd	getPollFd() const;

		int	readEvent(FdTable & fd_table);
		int	writeEvent(FdTable & fd_table);

		std::string	const &	getContent() const;
		void				setContent(std::string const & content);
		void				clearContent();
		void				swapContent(std::string & content);

	private:
		std::string	_content;
};
