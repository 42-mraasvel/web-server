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
		int	closeEvent();

		std::string	const &	getContent() const;
		void				setContent(std::string const & content);
		void				appendContent(std::string const & content);
		bool				getEventComplete() const;

	private:
		std::string	_content;
		bool		_event_complete;
};
