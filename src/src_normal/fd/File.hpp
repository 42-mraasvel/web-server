#pragma once
#include "AFdInfo.hpp"
#include <string>

class Client;

class File : public AFdInfo
{
	private:
		File(File const & rhs);
		File & operator=(File const & rhs);

	public:
		File(int fd);
		struct pollfd	getPollFd() const;

		void	readEvent(FdTable & fd_table);
		void	writeEvent(FdTable & fd_table);
		void exceptionEvent(FdTable & fd_table);

		std::string	const &	getContent() const;
		void				appendContent(std::string & content);

	/* Debugging */
	public:
		std::string getName() const;
	
	private:
		void markError(FdTable & fd_table);
		void markFinished(FdTable & fd_table, AFdInfo::Flags flag);

	private:
		std::string	_content;
};
