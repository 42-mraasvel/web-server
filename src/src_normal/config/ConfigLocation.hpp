#pragma once
#include <vector>
#include <map>
#include <string>


class ConfigLocation
{
	public:
		ConfigLocation(std::string path);
		void	addRoot(std::string root);
		void	addClientBodySize(int client_body_size);
		void	addAllowedMethods(std::string method);
		void	addAutoIndex(int status);
		void	addIndex(std::string index);
		void	addCgi(std::string cgi);

	private:
		std::string 				_path;
		std::string					_root;
		std::vector<std::string>	_index;
		std::vector<std::string>	_allowed_methods;
		std::vector<std::string>	_cgi;
		int							_client_body_size;
		int							_autoindex_status;

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
