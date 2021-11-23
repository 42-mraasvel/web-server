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
		void				appendContent(std::string & content);

	/* Debugging */
	public:
		std::string getName() const;

	private:
		std::string	_content;
};
