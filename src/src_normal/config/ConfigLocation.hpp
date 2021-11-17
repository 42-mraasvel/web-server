#pragma once
#include <vector>
#include <map>
#include <string>

enum location_flag {NONE, EQUAL};

typedef struct s_LocationBlock
{
	std::string											_path;
	std::string											_root;
	std::pair<int, std::string>							_return;
	std::vector<std::string>							_index;
	std::vector<std::string>							_allowed_methods;
	std::vector<std::pair<std::string, std::string> >	_cgi;
	bool												_autoindex_status;
	location_flag										_location_flag;

}				LocationBlock;

class ConfigLocation
{
	public:
		ConfigLocation(std::string path);
		void	addRoot(std::string root);
		void	addClientBodySize(size_t client_body_size);
		void	addAllowedMethods(std::string method);
		void	addAutoIndex(int status);
		void	addIndex(std::string index);
		void	addCgi(std::string extention, std::string path);
		void	addReturn(int code, std::string path);
		void	addLocationFlag(location_flag flag);
	// Utility
		LocationBlock* getLocationBlock();
		void	initLocationBlock();

		//TODO: only for ConfigResolver, to delete
		std::string const &					getPath() const;
		std::vector<std::string> const &	getIndex() const;
		std::vector<std::string> const &	getAllowedMethods() const;
		int 						getAutoIndexStatus() const;
		std::string const & 				getRoot() const;
		bool				auto_index_status;
		bool				redirect;

	private:
		std::string 				_path;
		std::string					_root;
		std::vector<std::string>	_index;
		std::vector<std::string>	_allowed_methods;
		std::vector<std::pair<std::string, std::string> >	_cgi;
		size_t						_client_body_size;
		bool						_autoindex_status;
		std::pair<int, std::string>	_return;
		location_flag				_location_flag;
		LocationBlock				*_location_block;

	/* Debugging */
	public:
		void print() const;

	private:
		void printPath() const;
		void printRoot() const;
		void printIndex() const;
		void printAutoIndex() const;
		void printAllowedMethods() const;
		void printClientBodySize() const;
		void printCgi() const;
};
