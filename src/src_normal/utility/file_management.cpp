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

}
