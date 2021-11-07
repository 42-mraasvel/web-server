#pragma once
#include "config/ConfigServer.hpp"
#include "FdTable.hpp"

class AFdInfo
{
	public:
		enum EventTypes
		{
			WRITING,
			WAITING,
			READING
		};
		//TODO: change from FILE_x to just x: COMPLETE, ERROR, START etc, use it for CGI as well
		//Also make flag private and create a getter/setter
		enum Flags
		{
			ACTIVE,
			FILE_START,
			FILE_COMPLETE,
			FILE_ERROR,
			TO_ERASE
		};
		Flags		flag;

	public:
		virtual	~AFdInfo();
		virtual struct pollfd getPollFd() const = 0;
		virtual	int		writeEvent(FdTable & fd_table) = 0;
		virtual	int		readEvent(FdTable & fd_table) = 0;
		virtual void	closeEvent(FdTable & fd_table); // Make virtual later
		virtual	void	updateEvents(AFdInfo::EventTypes type, FdTable & fd_table);
		virtual	void	update(FdTable & fd_table);

		AFdInfo();
		AFdInfo(int fd);
		int			getFd() const;
		std::size_t getIndex() const;
		void 		setIndex(std::size_t index);

		void		setToErase();
		void		closeFd();
		void		closeFd(FdTable & fd_table);
	
	/* Debugging, dispatchFd output */
		virtual std::string getName() const = 0;

	protected:
		std::size_t	_index;
		int			_fd;
};
