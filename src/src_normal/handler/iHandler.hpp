#pragma once
#include <string>

class FdTable;

struct Request;

class HeaderField;

class iHandler
{
	public:
		virtual				~iHandler() {}
		virtual int			executeRequest(FdTable & fd_table, Request & request) = 0;
		virtual void		update(std::string & response_body) = 0;
		virtual void		exceptionEvent() = 0;
		virtual bool		isComplete() const = 0;
		virtual bool		isError() const = 0;
		virtual int			getStatusCode() const = 0;
		virtual bool		isReadyToWrite() const = 0;
		virtual void		setSpecificHeaderField(HeaderField & header_field, bool content_type_fixed) = 0;
};
