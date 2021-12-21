#include "catch.hpp"
#include "config/Config.hpp"
#include "settings.hpp"
#include "utility/utility.hpp"
#include <iostream>
#include <dirent.h>
#include <vector>
#include <filesystem>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>

namespace __TEST__
{

static bool isDirectory(const std::string& filename) {
	struct stat s;
	if (stat(filename.c_str(), &s) == 0)  {
		if (s.st_mode & S_IFDIR) {
			return true;
		}
	}
	return false;
}

static void iterateDirectory(const std::string& dirname, std::vector<std::string>& files, const std::string& prefix) {
	DIR* dir = opendir(dirname.c_str());
	if (!dir) {
		perror("opendir");
		return;
	}

	while (dirent* dent = readdir(dir)) {
		const std::string name =  prefix + dent->d_name;
		if (name.find("/..") == name.size() - 3 || name.find("/.") == name.size() - 2) {
			continue;
		}
		if (isDirectory(name)) {
			iterateDirectory(name, files, name + "/");
		} else {
			files.push_back(name);
		}
	}
	closedir(dir);
}

}

std::vector<std::string> getFilesRecursively(const std::string& dirname) {
	std::vector<std::string> files;
	if (dirname.back() != '/') {
		__TEST__::iterateDirectory(dirname, files, "./" + dirname + "/");
	} else {
		__TEST__::iterateDirectory(dirname, files, "./" + dirname);
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
			std::cout << "TESTING: " << *it << std::endl;
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