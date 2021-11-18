#pragma once
#include <map>
#include <utility>
#include <string>
#include "config/Config.hpp"

struct Request;

class ConfigResolver
{
	public:
		ConfigResolver();

	public:
		//TODO: to evaluate typedef for config
		typedef	Config::address_map							MapType;
		typedef	Config::ip_host_pair						AddressType;
		typedef	Config::server_block_vector					ServerVector; 
		typedef std::vector< LocationBlock * >				LocationVectorType;
		typedef std::vector<std::string>					StringVectorType;
		typedef	std::vector<std::pair<int, std::string> >	ErrorPageType;
		typedef std::pair< int, std::string>				RedirectType;
		enum ConfigResult
		{
			START,
			LOCATION_RESOLVED,
			REDIRECT,
			AUTO_INDEX_ON,
			NOT_FOUND
		};

	/* general resolve */
	public:
		void	resolution(MapType const & map, AddressType const & request_address, std::string const & request_host, std::string const & request_target);
	private:
		ServerVector	resolveAddress(AddressType client_address, MapType const & map);
		void				setAddress(AddressType const & client_address, AddressType & address, MapType const & map);
		ServerBlock*	resolveHost(std::string const & host, ServerVector const & servers);
		bool				isMatchEmpty(std::string const & host, ServerVector const & servers, ServerVector::const_iterator & it_matched);
		bool					isServerNameEmpty(StringVectorType const & server_names);
		bool				isMatchExactName(std::string const & host, ServerVector const & servers, ServerVector::const_iterator & it_matched);
		bool					isServerNameExactMatch(std::string const & host, StringVectorType const & server_names);
		bool				isMatchFrontWildcard(std::string const & host, ServerVector const & servers, ServerVector::const_iterator & it_matched);
		bool					isServerNameFrontWildcardMatch(std::string const & host, StringVectorType const & server_names, std::string & longest_match);
		bool					isFrontWildCard(std::string const & string);
		bool					isHostMatchFrontWildCard(std::string const & host, std::string const & wildcard);
		bool				isMatchBackWildcard(std::string const & host, ServerVector const & servers, ServerVector::const_iterator & it_matched);
		bool					isServerNameBackWildcardMatch(std::string const & host, StringVectorType const & server_names, std::string & longest_match);
		bool					isBackWildCard(std::string const & string);
		bool					isHostMatchBackWildCard(std::string const & host, std::string const & wildcard);		
		ServerBlock*		resolveDefaultHost(ServerVector const & servers);
		LocationBlock*	resolveLocation(std::string const & target, LocationVectorType const & locations);
		bool				isMatchLocation(std::string const & target, LocationVectorType const & locations, LocationVectorType::const_iterator & it_matched);
		bool				isPrefixMatch(std::string const & target, std::string const & location);
		bool				isTargetDirectory(std::string const & target);
		LocationBlock*		resolveIndex(LocationVectorType::const_iterator it_matched, std::string const & target, LocationVectorType const & locations);
		LocationBlock*			resolveIndexFile(StringVectorType indexes, std::string const & target, LocationVectorType const & locations);
		LocationBlock*			resolveAutoIndex(LocationVectorType::const_iterator it_matched);
		ConfigResolver::ConfigResult	getResult(LocationBlock* location);
		bool								isReturnOn(LocationBlock* location) const;
		bool								isAutoIndexOn(LocationBlock* location) const;
		std::string						getResolvedFilePath();

	/* resolve error page */
	public:
		int	resolveErrorPage(int error_code, std::string & file_path);
	private:
		int		findErrorFilePath(std::string const & error_uri, std::string & file_path);

	public:
		ServerBlock*	resolved_server;
		LocationBlock*	resolved_location;
		std::string		resolved_target;
		ConfigResult	result;
		std::string		resolved_file_path;


	// debug 
	private:
		void	print() const;
		void		printSolutionServer(ServerBlock * server) const;
		void		printSolutionLocation(LocationBlock * location) const;

};
