#pragma once

# include "parser/Request.hpp"
# include "parser/HeaderField.hpp"

# include <vector>
# include <utility>

class CgiHandler
{
	private:
		typedef std::pair<std::string, std::string> MetaVariableType;
		typedef std::vector<MetaVariableType> MetaVariableContainerType;

	public:
		enum Status {
			INCOMPLETE,
			COMPLETE
		};
	public:
		CgiHandler();

		bool isCgi(Request* request);
		int execute(Request* request);
		bool isComplete() const;

		const std::string& getContent() const;
		const HeaderField& getHeaderField() const;
		int getStatusCode() const;
		Status getStatus() const;

		void generateMessageBody(std::string & message_body);

		void clearContent();

	/* Debugging */
	public:
		void print() const;

	private:
		Status		_status;
		int			_status_code;
		std::string _message_body;
		HeaderField _header;

	private:
		std::string _target;
		MetaVariableContainerType _meta_variables;
};
