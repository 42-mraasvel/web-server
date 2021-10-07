#pragma once

#include <iostream>

class Config
{
	private:
		std::string	_path;
		std::string _method;
		int			_client_body_buffer_size;
		int			_listen;

	public:
		Config();
		~Config();

		void	setPath(std::string const & path);
		void	setMethod(std::string const & method);
		void	setClientBuffer(int bufferSize);
		void	setListenPort(int port);

		std::string getPath(void);
		std::string getMethod(void);
		
		int	getClientBuffer(void);
		int	getListenPort(void);

};








