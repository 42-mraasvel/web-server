#pragma once
#include "Request.hpp"
#include "ChunkedParser.hpp"
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
		typedef std::queue<Request*> queue_type;

	private:
		enum MessageBodyType
		{
			CHUNKED,
			LENGTH,
			NOT_PRESENT
		};

	public:

		RequestParser();
		~RequestParser();

		int					parse(std::string const & buffer);
		Request* getNextRequest();
	
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

	/* Helpers */
		typedef bool (*IsFunctionT)(char);
		int parseWhiteSpace();
		MethodType getMethodType(std::string const & s) const;
		void skip(IsFunctionT condition);

	/* Request Related */

		int parseHeader();
		int checkHeaderFields();
		int parseContentLength(std::string const & value);
		int parseTransferEncoding(std::string const & value);

		void clearToIndex();
		void clearToEoHeader();
		void resetBuffer();

		void newRequest();
		bool leftOverRequest() const;
		bool checkHeaderEnd();

		int parseMessageBody();
		int parseContent();
		int parseChunked();

		int delimitRequest(Request::RequestStatus status);

	private:
		queue_type	_requests;
		int			_status_code;
		Request*	_request;
		std::size_t	_index;

		std::string	_buffer;

	/* For Message-Body Parsing */
		ChunkedParser			_chunked_parser;
		MessageBodyType			_body_type;
		std::size_t				_remaining_content;
};
