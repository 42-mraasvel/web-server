#pragma once
#include <vector>
#include <map>
#include <string>

typedef struct s_LocationBlock
{
	std::string											_path;
	std::string											_root;
	std::vector<std::string>							_index;
	std::vector<std::string>							_allowed_methods;
	std::vector<std::pair<std::string, std::string> >	_cgi;
	int													_autoindex_status;

}				LocationBlock;

class ConfigLocation
{
	public:
		ConfigLocation(std::string path);
		void	addRoot(std::string root);
		void	addAllowedMethods(std::string method);
		void	addAutoIndex(int status);
		void	addIndex(std::string index);
		void	addCgi(std::string extention, std::string path);

	// Utility
		LocationBlock getLocationBlock();
		void	initLocationBlock();

	private:
		std::string 				_path;
		std::string					_root;
		std::vector<std::string>	_index;
		std::vector<std::string>	_allowed_methods;
		std::vector<std::pair<std::string, std::string > >	_cgi;
		int							_autoindex_status;
		LocationBlock				_location_block;

	/* Debugging */
	public:
		void print() const;

	private:
		void printPath() const;
		void printRoot() const;
		void printIndex() const;
		void printAutoIndex() const;
		void printAllowedMethods() const;
		void printCgi() const;
};
