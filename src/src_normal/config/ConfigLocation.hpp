#pragma once
#include <vector>
#include <map>
#include <string>


class ConfigLocation
{
	public:
		ConfigLocation(std::string path);
		void	addRoot(std::string root);
		void	addClientBodySize(size_t client_body_size);
		void	addAllowedMethods(std::string method);
		void	addAutoIndex(int status);
		void	addIndex(std::string index);
		void	addCgi(std::string cgi);

		//TODO: only for ConfigResolver, to delete
		std::string const &					getPath() const;
		std::vector<std::string> const &	getIndex() const;
		std::vector<std::string> const &	getAllowedMethods() const;
		int	const & 						getAutoIndexStatus() const;
		std::string const & 				getRoot() const;
		bool				auto_index_status;
		bool				redirect;

	private:
		std::string 				_path;
		std::string					_root;
		std::vector<std::string>	_index;
		std::vector<std::string>	_allowed_methods;
		std::vector<std::string>	_cgi;
		size_t						_client_body_size;
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
