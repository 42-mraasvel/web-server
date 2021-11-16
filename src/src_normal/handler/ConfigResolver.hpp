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
		ConfigResolver();

	public:
		//TODO: to evaluate typedef for config
		typedef	std::vector< ConfigServer * >				ServerVector; 
		typedef std::map< Request::Address, ServerVector >	ConfigMap;
		typedef std::vector<std::string>					StringVector;
		typedef std::vector< ConfigLocation * >				LocationVector;
		typedef std::pair< int, std::string>				RedirectInfo;
		typedef	std::vector<std::pair<int, std::string> >	ErrorPageInfo;
		enum ConfigResult
		{
			START,
			LOCATION_RESOLVED,
			REDIRECT,
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
		void			setResult();
		void				scanLocation();
		void					setResolvedFilePath();
		void					setAutoIndexPage();
		void					setRedirect();

	public:
		bool	isDefaultErrorPage(int error_code) const;
		void	resolveErrorPage(int error_code);
		std::string	getErrorPageUri(int error_code) const;
		int			resolveErrorPage(int error_code, std::string & file_path);

	public:
		std::string		auto_index_page;
		RedirectInfo	redirect_info;
		std::string		resolved_target;
		std::string		resolved_host;
		std::string		resolved_file_path;
		ConfigServer*	resolved_server;
		ConfigLocation*	resolved_location;

	private:
		bool			auto_index;
		bool			redirect;

	/* debug */
	private:
		void	createConfigMap(ConfigMap & map);
		void	createServers(ServerVector & servers, LocationVector const & locations);
		void	createLocations(LocationVector & locations);
		void	print() const;
		void		printSolutionServer(ConfigServer * server) const;
		void		printSolutionLocation(ConfigLocation * location) const;
		void		printAutoIndexPage() const;
};
