#pragma once
#include "utility/SmartPointer.hpp"
#include <vector>

class AFdInfo;

class FdTable
{
	public:
		typedef SmartPointer<AFdInfo>					AFdPointer;
		typedef std::vector<struct pollfd>				PollFdTable;
		typedef std::vector<AFdPointer>					FdVector_t;
		typedef std::pair<struct pollfd&, AFdPointer>	pair_t;
		typedef std::size_t								size_type;

	private:
		FdTable(FdTable const & rhs);
		FdTable & operator=(FdTable const & rhs);

	public:
		FdTable();
		~FdTable();
		void	insertFd(AFdPointer info);
		void	eraseFd(size_type index);
		size_type	size();
		struct pollfd* getPointer();
		pair_t operator[](size_type index);

	private:

	private:
		PollFdTable		_pollfd_table;
		FdVector_t		_fd_info_table;
	
	/* Debuging */
	public:
		void print() const;
};
