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
		Response(Request const & request);

	public:
		void	executeRequest(FdTable & fd_table, Request & request);
		void	update(FdTable & fd_table);
		void	generateResponse(std::string & append_to);

		bool	isReadyToWrite() const;
		bool	isComplete() const;

		Method::Type	getMethod() const;
		bool			getCloseConnectionFlag() const;

	private:
		ResponseGenerator	_generator;
		ResponseProcessor	_processor;

};
