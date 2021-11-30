#pragma once
#include "utility/SmartPointer.hpp"
#include <string>
#include <vector>
#include "parser/HeaderField.hpp"
#include "webserver/MethodType.hpp"
#include "ConfigResolver.hpp"
#include "ResponseGenerator.hpp"
#include "ResponseProcessor.hpp"

struct	Request;

class	FdTable;

class Response
{
	public:
		typedef HeaderField::const_iterator	header_iterator;
		typedef	std::vector<std::string>::const_iterator		method_iterator;

	public:
		Response(Request const & request);

	public:
		void	executeRequest(FdTable & fd_table, Request & request);
		void	update(FdTable & fd_table);
		void	generateResponse(std::string & append_to);

	/* utility */
	public:
		bool	isReadyToWrite() const;
		bool	isComplete() const;
		Method::Type	getMethod() const;
		bool			getCloseConnectionFlag() const;

	// TODO: to delete
		void	print() const;

	private:
		ResponseGenerator	_generator;
		ResponseProcessor	_processor;

};
