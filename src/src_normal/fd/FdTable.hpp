#pragma once
#include <vector>
#include <utility>

class AFdInfo;

class FdTable
{
	public:
		typedef std::vector<struct pollfd>				PollFdTable;
		typedef std::vector<AFdInfo *>					FdVector_t;
		typedef std::pair<struct pollfd&, AFdInfo *>	pair_t;
		typedef std::size_t								size_type;
	
	public:
		~FdTable();
		void	insertFd(AFdInfo * info);
		void	eraseFd(size_type index);
		size_type	size();
		struct pollfd* getPointer();
		pair_t operator[](size_type index);

	private:

	private:
		std::vector<struct pollfd>	_pollfd_table;
		std::vector<AFdInfo *>		_fd_info_table;
	
	/* Debuging */
	public:
		void print() const;
};
