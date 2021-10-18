#pragma once
#include "Request.hpp"
#include <queue>

class RequestParser
{
	public:
		enum ParseResult
		{
			REQUEST_COMPLETE,
			CONT_READING,
			BAD_REQUEST
		};

		struct HttpVersion
		{
			HttpVersion();
			HttpVersion(int maj, int min);

			int major;
			int minor;
		};

		typedef Request::header_field_t header_field_t;

	public:

		RequestParser();
		~RequestParser();

		int					parse(std::string const & buffer);
		Request* getNextRequest();


	// TO BE REMOVED
		int parseHeader(std::string const &request); // TODO: make private
		MethodType			getMethod() const;
		const std::string&	getTargetResource() const;
		HttpVersion			getHttpVersion() const;
		header_field_t&		getHeaderFields();
		const std::string&	getMessageBody() const;
	
	private:
	/* Request Line Parsing */
		int parseRequestLine();
		int parseSpace();
		int parseMethod();
		int parseTargetResource();
		bool skipAbsolutePath();
		bool skipQuery();
		int parseVersion();
		bool parseMajorVersion();
		bool parseMinorVersion();

	/* Header Field Parsing */

		int parseHeaderFields();

		int parseFieldName(std::string & key);
		int parseColon();
		int parseFieldValue(std::string & value);
		int parseEndLine();

	/* Message Body Parsing */

		int parseMessageBody(std::string const & request);

	/* Helpers */
		typedef bool (*IsFunctionT)(char);
		int parseWhiteSpace();
		MethodType getMethodType(std::string const & s) const;
		void skip(IsFunctionT condition);

		void resetParser();

	/* Request Related */

		int parseHeader2();
		int parseMessageBody2();

		void clearToIndex();
		void clearToEoHeader();
		void newRequest();
		bool leftOverRequest() const;
		bool checkHeaderEnd();

	public:
	/* Debugging */
		std::string getMethodString() const;
		void print() const;

	private:
		std::queue<Request*>	_requests;
		int						_status_code;
		Request*				_request;
		std::size_t				_index;

		std::string				_buffer;

		enum MethodType	_method;
		std::string		_target_resource;
		HttpVersion		_version;
		header_field_t	_header_fields;
		std::string		_message_body;

};
