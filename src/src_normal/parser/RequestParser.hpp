#pragma once
#include <string>
#include <map>

class RequestParser
{
	public:
		enum MethodType
		{
			GET,
			POST,
			DELETE,
			OTHER
		};

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

		typedef std::map<std::string, std::string> header_field_t;

	public:

		RequestParser();

		int					parseHeader(std::string const &request);
		enum MethodType		getMethod() const;
		const std::string&	getTargetResource() const;
		HttpVersion			getHttpVersion() const;
		header_field_t&		getHeaderFields();
		const std::string&	getMessageBody() const;
	
	private:
	/* Request Line Parsing */
		int parseRequestLine(std::string const & request);
		int parseSpace(std::string const & s);
		int parseMethod(std::string const & s);
		int parseTargetResource(std::string const & s);
		bool skipAbsolutePath(std::string const & s);
		bool skipQuery(std::string const & s);
		int parseVersion(std::string const & s);
		bool parseMajorVersion(std::string const & s);
		bool parseMinorVersion(std::string const & s);

	/* Header Field Parsing */

		int parseHeaderFields(std::string const & request);

		int parseFieldName(std::string const & request, std::string & key);
		int parseColon(std::string const & request);
		int parseFieldValue(std::string const & request, std::string & value);
		int parseEndLine(std::string const & request);

	/* Message Body Parsing */

		int parseMessageBody(std::string const & request);

	/* Helpers */
		typedef bool (*IsFunctionT)(char);
		int parseWhiteSpace(std::string const & request);
		enum MethodType getMethodType(std::string const & s) const;
		void skip(std::string const & s, IsFunctionT condition);

		void resetParser();

	public:
	/* Debugging */
		std::string getMethodString() const;
		void print() const;

	private:
		enum MethodType	_method;
		std::string		_target_resource;
		HttpVersion		_version;
		header_field_t	_header_fields;
		std::string		_message_body;

		std::size_t		_index;
		int				_status_code;
};
