#pragma once
#include <string>
#include "parser/HeaderField.hpp"

struct	Response;

class ResponseGenerator
{
	private:
		typedef HeaderField::const_iterator		header_iterator;
		typedef	std::vector< std::string>		method_type;

	public:
		ResponseGenerator();
	private:
		ResponseGenerator(ResponseGenerator const & src);
		ResponseGenerator const & 	operator=(ResponseGenerator const & rhs);

	public:
		void	generateString(Response & response);
	private:
		void	evaluateEncoding(Response & response);
		bool		isReadyToBeChunked(Response const & response) const;

		void	generateChunkedResponse(Response & response);
		void	generateUnchunkedResponse(Response & response);

		void	setHeaderPart(Response & response);
		void		setStringStatusLine(Response & response);
		void		setHeaderField(Response & response);
		void			setDate(Response & response);
		void			setConnection(Response & response);
		void			setLocation(Response & response);
		void			setRetryAfter(Response & response);
		void			setAllow(Response & response);
		void			setTransferEncodingOrContentLength(Response & response);
		void				setContentLength(Response & response);
		void			setContentType(Response & response);
		void		setStringHeaderField(Response & response);

		void	appendMessageBody(Response & response);
		void	encodeMessageBody(Response & response);
};
