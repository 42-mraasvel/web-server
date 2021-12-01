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
			START,
			COMPLETE,
			ERROR,
			TO_ERASE
		};

	public:
		virtual	~AFdInfo();
		virtual struct pollfd getPollFd() const = 0;
		virtual	void	writeEvent(FdTable & fd_table) = 0;
		virtual	void	readEvent(FdTable & fd_table) = 0;
		virtual void	closeEvent(FdTable & fd_table); // Make pure virtual later
		virtual void	exceptionEvent(FdTable & fd_table);

		virtual	void	updateEvents(AFdInfo::EventTypes type, FdTable & fd_table);
		virtual	void	addEvents(AFdInfo::EventTypes type, FdTable & fd_table);
		virtual	void	removeEvents(AFdInfo::EventTypes type, FdTable & fd_table);
		virtual	void	update(FdTable & fd_table);

		AFdInfo();
		AFdInfo(int fd);
		int			getFd() const;
		std::size_t getIndex() const;
		void 		setIndex(std::size_t index);

		void		setToErase();
		void		closeFd();
		void		closeFd(FdTable & fd_table);
		Flags		getFlag() const;
		void		setFlag(AFdInfo::Flags flag);
	
	/* Debugging, dispatchFd output */
		virtual std::string getName() const = 0;

	protected:
		Flags		_flag;
		std::size_t	_index;
		int			_fd;
};
