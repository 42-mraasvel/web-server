#include "utility/utility.hpp"
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "settings.hpp"
#include <errno.h>

namespace WebservUtility
{

bool	isFileExist(std::string const & file_path)
{
	if (access(file_path.c_str(), F_OK) == ERR)
	{
		return false;
	}
	return true;
}

bool	isDirectoryExist(std::string const & directory_path)
{
	DIR *dir = opendir(directory_path.c_str());
	if (dir == NULL)
	{
		return false;
	}
	closedir(dir);
	return true;
}

/* 	path cannot be empty, and has to start with '/';
	only create directory with "/" at the end
*/
int	createDirectories(std::string const & path)
{
	size_t	i = 1;
	size_t	ret;
	int		new_dir_count = 0;
	while (i < path.size())
	{
		ret = path.find_first_of("/", i);
		if (ret == std::string::npos)
		{
			return new_dir_count;
		}
		else
		{
			std::string dir_path = path.substr(0, ret);
			if (!isDirectoryExist(dir_path))
			{
				if (mkdir(dir_path.c_str(), 0755) == ERR)
				{
					return ERR;
				}
				new_dir_count++;
			}
			i = ret + 1;
		}
	}
	return new_dir_count;
}
}
