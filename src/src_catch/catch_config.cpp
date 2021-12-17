#include "catch.hpp"
#include "config/Config.hpp"
#include "settings.hpp"
#include <iostream>
#include <dirent.h>
#include <vector>
#include <filesystem>

std::vector<std::string> getConfigs(std::string str)
{
	std::vector<std::string> ret;
	DIR *dir = opendir(str.c_str());
	struct dirent *file = readdir(dir);
	while (file)
	{
		std::string tmp = str;
		if (file->d_name[0] != '.')
		{
			tmp.append("/");
			tmp.append(file->d_name);
			ret.push_back(tmp);
		}
		file = readdir(dir); 
	}    
	closedir(dir);
	return (ret);
}

/*
(C++ 17)
https://en.cppreference.com/w/cpp/filesystem/recursive_directory_iterator
*/
std::vector<std::string> getFilesRecursively(const std::string& dirname) {
	std::vector<std::string> files;
	for (const auto& file : std::filesystem::recursive_directory_iterator(dirname)) {
		const std::string path = file.path();
		if (path.find("ignore") == std::string::npos) {
			files.push_back(path);
		}
	}
	return files;
}

TEST_CASE("Valid configurations", "[config]")
{
	std::vector<std::string> configs;
	configs = getFilesRecursively("src/src_catch/catch_config/valid");
	for (std::vector<std::string>::iterator it = configs.begin(); it != configs.end(); ++it)
	{
		SECTION((*it))
		{
			Config conf(*it);
			REQUIRE(conf.parser() == OK);
		}
	}
}


TEST_CASE("Invalid configurations", "[config]")
{
	std::vector<std::string> configs;
	configs = getFilesRecursively("src/src_catch/catch_config/invalid");
	for (std::vector<std::string>::iterator it = configs.begin(); it != configs.end(); ++it)
	{
		SECTION(*it)
		{
			Config conf(*it);
			REQUIRE(conf.parser() == ERR);
		}
	}
}