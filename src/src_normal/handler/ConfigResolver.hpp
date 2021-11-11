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
		ConfigResolver(std::string const & request_target);

	public:
		//TODO: to evaluate typedef for config
		typedef	std::vector< ConfigServer * >				ServerVector; 
		typedef std::map< Request::Address, ServerVector >	ConfigMap;
		typedef std::vector<std::string>					StringVector;
		typedef std::vector< ConfigLocation * >				LocationVector;
		enum ConfigResult
		{
			START,
			LOCATION_RESOLVED,
			AUTO_INDEX_ON,
			NOT_FOUND
		};
		ConfigResult	result;


	public:
		void	resolution(Request const & request);
	private:
		ServerVector	resolveAddress(ConfigMap map, Request::Address client_address);
		void				setAddress(ConfigMap const & map, Request::Address const & client_address, Request::Address & address);
		ConfigServer*	resolveHost(Request const & request, ServerVector const & servers);
		void				setHost(Request const & request);
		bool				isMatchEmpty(ServerVector const & servers, ServerVector::const_iterator & it_matched);
		bool					isServerNameEmpty(StringVector const & server_names);
		bool				isMatchExactName(ServerVector const & servers, ServerVector::const_iterator & it_matched);
		bool					isServerNameExactMatch(StringVector const & server_names);
		bool				isMatchFrontWildcard(ServerVector const & servers, ServerVector::const_iterator & it_matched);
		bool					isServerNameFrontWildcardMatch(StringVector const & server_names, std::string & longest_match);
		bool					isFrontWildCard(std::string const & string);
		bool					isHostMatchFrontWildCard(std::string const & wildcard);
		bool				isMatchBackWildcard(ServerVector const & servers, ServerVector::const_iterator & it_matched);
		bool					isServerNameBackWildcardMatch(StringVector const & server_names, std::string & longest_match);
		bool					isBackWildCard(std::string const & string);
		bool					isHostMatchBackWildCard(std::string const & wildcard);		
		ConfigServer*		resolveDefaultHost(ServerVector const & servers);
		ConfigLocation*	resolveLocation(std::string const & request_target, LocationVector const & locations);
		bool				isMatchLocation(std::string const & request_target, LocationVector const & locations, LocationVector::const_iterator & it_matched);
		bool				isPrefixMatch(std::string const & request_target, std::string const & location);
		bool				isTargetDirectory(std::string const & target);
		ConfigLocation*		resolveIndex(LocationVector::const_iterator it_matched, std::string const & request_target, LocationVector const & locations);
		ConfigLocation*			resolveIndexFile(StringVector indexes, std::string const & request_target, LocationVector const & locations);
		ConfigLocation*			resolveAutoIndex(LocationVector::const_iterator it_matched);

	/* utility */
	private:
		void	setResult();
		void	setResolvedFilePath();

	public:
		bool			auto_index;
		std::string		resolved_target;
		std::string		resolved_host;
		std::string		resolved_file_path;
		ConfigServer*	resolved_server;
		ConfigLocation*	resolved_location;

	/* debug */
	private:
		void	createConfigMap(ConfigMap & map);
		void	createServers(ServerVector & servers, LocationVector const & locations);
		void	printSolutionServer(ConfigServer * server);
		void	createLocations(LocationVector & locations);
		void	printSolutionLocation(ConfigLocation * location);
};
