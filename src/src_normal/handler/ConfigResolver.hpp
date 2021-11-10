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
		typedef std::vector< ConfigLocation * >				LocationVector;

		void	resolution(Request const & request);
	
	private:
		ServerVector	resolveAddress(ConfigMap map, Request::Address client_address);
		void				setAddress(ConfigMap const & map, Request::Address const & client_address, Request::Address & address);
		ConfigServer*	resolveHost(Request const & request, ServerVector const & servers);
		void				setHost(Request const & request, std::string & host);
		bool				isMatchEmpty(std::string const & host, ServerVector const & servers, ServerVector::const_iterator & it_matched);
		bool					isServerNameEmpty(StringVector const & server_names);
		bool				isMatchExactName(std::string const & host, ServerVector const & servers, ServerVector::const_iterator & it_matched);
		bool					isServerNameExactMatch(std::string const & host, StringVector const & server_names);
		bool				isMatchFrontWildcard(std::string const & host, ServerVector const & servers, ServerVector::const_iterator & it_matched);
		bool					isServerNameFrontWildcardMatch(std::string const & host, StringVector const & server_names, std::string & longest_match);
		bool					isFrontWildCard(std::string const & string);
		bool					isHostMatchFrontWildCard(std::string const & host, std::string const & wildcard);
		bool				isMatchBackWildcard(std::string const & host, ServerVector const & servers, ServerVector::const_iterator & it_matched);
		bool					isServerNameBackWildcardMatch(std::string const & host, StringVector const & server_names, std::string & longest_match);
		bool					isBackWildCard(std::string const & string);
		bool					isHostMatchBackWildCard(std::string const & host, std::string const & wildcard);		
		ConfigServer*		resolveDefaultHost(ServerVector const & servers);
		ConfigLocation*	resolveLocation(std::string const & request_target, LocationVector const & locations);
		bool				isMatchLocation(std::string const & request_target, LocationVector const & locations, LocationVector::const_iterator & it_matched);
		bool				isPrefixMatch(std::string const & request_target, std::string const & location);
		bool				isTargetDirectory(std::string const & target);
		ConfigLocation*		resolveIndex(StringVector indexes, std::string const & request_target, LocationVector const & locations);
		//TODO: to delte:
		void	createConfigMap(ConfigMap & map);
		void	createLocation(std::vector<ConfigLocation *> & locations);

	
	private:
		std::string		_new_target;
		std::string		_resolved_file_path;

};
