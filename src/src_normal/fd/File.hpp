#pragma once
#include "AFdInfo.hpp"
#include <string>

class Connection;

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
		void				appendToContent(std::string & to);
		void				appendFromContent(std::string & from);

	/* Debugging */
	public:
		std::string getName() const;
	
	private:
		void markError(FdTable & fd_table);
		void markFinished(FdTable & fd_table, AFdInfo::Flags flag);

	private:
		std::string	_content;
};
