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
		virtual void		update() = 0;
		virtual bool		isComplete() const = 0;
		virtual bool		isError() const = 0;
		virtual void		setMessageBody(std::string & message_body) = 0;
		virtual int			getStatusCode() const = 0;
		virtual bool		isReadyToWrite() const = 0;
		virtual void		setSpecificHeaderField(HeaderField & header_field) = 0;
		virtual std::string	resolvedRequestTarget(Request const & request) = 0;
};
