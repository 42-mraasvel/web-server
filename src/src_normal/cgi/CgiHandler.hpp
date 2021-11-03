#pragma once

# include "parser/Request.hpp"
# include "parser/HeaderField.hpp"

class CgiHandler
{
	public:
		enum Status {
			INCOMPLETE,
			COMPLETE
		};
	public:
		CgiHandler();

		bool isCgi(Request* request) const;
		int execute(Request* request);
		bool isComplete() const;

		const std::string& getContent() const;
		const HeaderField& getHeaderField() const;
		int getStatusCode() const;
		Status getStatus() const;

		void clearContent();

	private:
		Status		_status;
		int			_status_code;
		std::string _message_body;
		HeaderField _header;
};
