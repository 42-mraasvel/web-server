#pragma once
#include <map>
#include <utility>
#include <string>
#include "config/Config.hpp"
#include "parser/Request.hpp"

struct Request;

class ConfigResolver
{
	public:
		//TODO: to evaluate typedef for config
		typedef	std::vector< ConfigServer * >				ServerVector; 
		typedef std::map< Request::Address, ServerVector >	ConfigMap;
		typedef std::vector<std::string>					StringVector;
		
		void	resolution(Request const & request);
	
	private:
		ServerVector	resolveAddress(ConfigMap map, Request::Address client_address);
		void				setAddress(ConfigMap const & map, Request::Address const & client_address, Request::Address & address);
		ConfigServer*	resolveHost(Request const & request, ServerVector const & servers);
		void				setHost(Request const & request, std::string & host);
		bool				isHostMatched(std::string const & host, StringVector const & server_names);
		
};
