#pragma once
#include "config/ConfigServer.hpp"
#include "FdTable.hpp"

class AFdInfo
{
	public:
		enum Flags
		{
			ACTIVE,
			TO_ERASE
		};

	public:
		virtual	~AFdInfo();
		virtual struct pollfd getPollFd() const = 0;
		virtual	int	writeEvent(FdTable & fd_table) = 0;
		virtual	int	readEvent(FdTable & fd_table) = 0;
		virtual int closeEvent() = 0;

		AFdInfo();
		AFdInfo(int fd);
		int	getFd() const;
		std::size_t getIndex() const;
		void setIndex(std::size_t index);

	protected:
		std::size_t	_index;
		int			_fd;
};
