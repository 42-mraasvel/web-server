#pragma once
#include "ConfigInfo.hpp"
#include "config/Config.hpp"
#include <map>
#include <utility>
#include <string>

struct Request;

class ConfigResolver
{
	public:
		//TODO: to evaluate typedef for config
		typedef	Config::address_map							MapType;
		typedef	Config::ip_host_pair						AddressType;
		typedef	Config::server_block_vector					ServerVector; 
		typedef std::vector< LocationBlock * >				LocationVectorType;
		typedef std::vector<std::string>					StringVectorType;
		typedef	std::vector<std::pair<int, std::string> >	ErrorPageType;
		typedef std::pair< int, std::string>				RedirectType;

	public:
		//TODO: remove DEFAULT constructor, it's here because of conflict with Response
		ConfigResolver();
		ConfigResolver(AddressType address, MapType* config_map);

	/* general resolve */
	public:
		//TODO: remove resolution with MapType, AddressType, response conflict
		void	resolution(MapType const & map, AddressType const & request_address, std::string const & request_host, std::string const & request_target);
		void	resolution(std::string const & request_host, std::string const & request_target);
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
		LocationBlock*	resolveLocationResult(std::string const & target, LocationVectorType const & locations);
		LocationBlock*		resolveLocation(std::string const & target, LocationVectorType const & locations);
		bool					isMatchLocation(std::string const & target, LocationVectorType const & locations, LocationVectorType::const_iterator & it_matched);
		bool					isPrefixMatch(std::string const & target, std::string const & location);
		bool					isTargetDirectory(std::string const & target);
		LocationBlock*			resolveIndex(LocationVectorType::const_iterator it_matched, std::string const & target, LocationVectorType const & locations);
		LocationBlock*				resolveIndexFile(StringVectorType indexes, std::string const & target, LocationVectorType const & locations);
		LocationBlock*				resolveAutoIndex(LocationVectorType::const_iterator it_matched);
		ConfigInfo::ConfigResult	getResult(LocationBlock* location);
		bool								isReturnOn(LocationBlock* location) const;
		bool								isAutoIndexOn(LocationBlock* location) const;
		std::string						getResolvedFilePath();

	/* resolve error page */
	public:
		int	resolveErrorPage(int error_code, std::string & file_path);
	private:
		int		findErrorFilePath(std::string const & error_uri, std::string & file_path);

	public:
		ConfigInfo const & getConfigInfo() const;

	private:
		//TODO: should this be a const pointer?
		AddressType	_address;
		MapType*	_config_map;
	public:
		//TODO: make private member instead, public for response conflict
		ConfigInfo	info;


	// debug 
	private:
		void	print() const;
		void		printSolutionServer(ServerBlock * server) const;
		void		printSolutionLocation(LocationBlock * location) const;

};
