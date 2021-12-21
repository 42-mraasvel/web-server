#pragma once
#include "ConfigInfo.hpp"
#include "Config.hpp"
#include "webserver/MethodType.hpp"
#include <map>
#include <utility>
#include <string>

struct Request;

class ConfigResolver
{
	public:
		typedef	Config::address_map							MapType;
		typedef	Config::ip_host_pair						AddressType;
		typedef	Config::server_block_vector					ServerVector; 
		typedef std::vector< ConfigInfo::location_block_pointer >				LocationVectorType;
		typedef std::vector<std::string>					StringVectorType;
		typedef	std::vector<std::pair<int, std::string> >	ErrorPageType;
		typedef std::pair< int, std::string>				RedirectType;

	public:
		ConfigResolver(MapType const * config_map);
		ConfigResolver(ConfigInfo::server_block_pointer server);

	/* general resolve */
	public:
		void	resolution(Request const & request, std::string const & request_host);
	private:
		ServerVector	resolveAddress(AddressType interface_address, MapType const & map);
		void				setAddress(AddressType const & interface_address, AddressType & address, MapType const & map);
		ConfigInfo::server_block_pointer	resolveHost(std::string const & host, ServerVector const & servers);
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
		ConfigInfo::server_block_pointer		resolveDefaultHost(ServerVector const & servers);
		ConfigInfo::location_block_pointer	resolveLocationResult(std::string const & target, LocationVectorType const & locations);
		ConfigInfo::location_block_pointer		resolveLocation(std::string const & target, LocationVectorType const & locations);
		bool					isMatchLocation(std::string const & target, LocationVectorType const & locations, LocationVectorType::const_iterator & it_matched);
		bool					isExactMatch(std::string const & target, std::string const & location) const;
		bool					isPrefixMatch(std::string const & target, std::string const & location) const;
		bool					isTargetDirectory(std::string const & target);
		ConfigInfo::location_block_pointer			resolveIndex(LocationVectorType::const_iterator it_matched, std::string const & target, LocationVectorType const & locations);
		ConfigInfo::location_block_pointer				resolveIndexFile(StringVectorType indexes, std::string const & target, LocationVectorType const & locations);
		ConfigInfo::location_block_pointer				resolveAutoIndex(LocationVectorType::const_iterator it_matched);
		ConfigInfo::ConfigResult	getResult(ConfigInfo::location_block_pointer location);
		bool								isReturnOn(ConfigInfo::location_block_pointer location) const;
		bool								isAutoIndexOn() const;
		std::string						getResolvedFilePath();

	/* resolve error page */
	public:
		int	resolveErrorPage(int error_code);
	private:
		int		findErrorFilePath(std::string const & error_uri);

	public:
		ConfigInfo const & getConfigInfo() const;

	private:
		MapType const *	_config_map;
		ConfigInfo		info;
		bool			_auto_index_on;


	// debug 
	private:
		void	print() const;
		void		printSolutionServer(ConfigInfo::server_block_pointer server) const;
		void		printSolutionLocation(ConfigInfo::location_block_pointer location) const;

};
