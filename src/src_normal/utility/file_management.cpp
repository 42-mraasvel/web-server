#include "utility/utility.hpp"
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "settings.hpp"

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


/* 	path cannot be empty, and has to start with '/';
	only create directory with "/" at the end
*/
// TODO: to make
//	struct dirent *entry;
//	DIR *dir = opendir(path.c_str());
//	if (dir == NULL)
//	{
//	   return ERR;
//	}
int	createDirectories(std::string const & path)
{
	if (path.empty() || path[0] != '/')
	{
		return ERR;
	}
	return OK;
}
}
