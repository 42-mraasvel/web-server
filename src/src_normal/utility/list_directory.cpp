#include "utility/utility.hpp"
#include <dirent.h>
#include <sys/types.h>
#include <time.h>
#include <sys/stat.h>
#include "settings.hpp"

namespace WebservUtility
{

int	getFileCreationTime(std::string const & path, std::string & creation_time)
{
	struct stat attr;
	if (stat(path.c_str(), &attr) == ERR)
		return ERR;
	creation_time = ctime(&attr.st_mtime);
	return OK;
}

std::string	getFileLine(struct dirent *entry, std::string const & path)
{
	std::string line;
	std::string file_name = entry->d_name;
	if (entry->d_type == DT_DIR)
	{
		file_name += "/";
	}
	std::string file_path = path + std::string(entry->d_name);
	std::string creation_time;
	if (entry->d_type == DT_REG && getFileCreationTime(file_path, creation_time) == ERR)
	{
		return line;
	}
	line = "<a href=\"" + file_name + "\">" + file_name + "</a>	" + creation_time;
	if (entry->d_type == DT_DIR)
	{
		line += "\n";
	}
	return line;
}

int list_directory(std::string const & directory, std::string const & path, std::string & content)
{
	struct dirent *entry;
	DIR *dir = opendir(path.c_str());
	if (dir == NULL)
	{
	   return ERR;
	}
	std::string title = "Index of " + directory;
	content = "<html>\n<head>\n<title>" + title + "</title>\n</head>\n<body>\n<h1>" + title + "</h1><hr><pre>\n";
	while ((entry = readdir(dir)) != NULL)
	{
	   if (std::string(entry->d_name) == "..")
	   {
		   continue ;
	   }
	   std::string line = getFileLine(entry, path);
	   if (line.empty())
	   {
  			closedir(dir);
			return ERR;
	   }
		content.append(line);
	}
	content.append("</pre><hr></body>\n</html>\n");
	closedir(dir);
	return OK;
}

}
