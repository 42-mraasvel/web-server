#include "utility/utility.hpp"
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "settings.hpp"

namespace WebservUtility
{

bool	isFileExisted(std::string const & file_path)
{
	if (access(file_path.c_str(), F_OK) == ERR)
	{
		return false;
	}
	return true;
}

bool	isDirectoryExisted(std::string const & directory_path)
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
	size_t		i = 1;
	size_t		ret;
	int			new_dir_count = 0;
	std::string	dir_path;
	
	while (i < path.size())
	{
		ret = path.find_first_of("/", i);
		if (ret == std::string::npos)
		{
			break;
		}
		else
		{
			dir_path = path.substr(0, ret);
			if (!isDirectoryExisted(dir_path))
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
	if (!dir_path.empty() && access(dir_path.c_str(), W_OK) == ERR)
	{
		return ERR;
	}
	return new_dir_count;
}
}
