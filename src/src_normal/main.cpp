#include "settings.hpp"
#include "utility/Output.hpp"
#include "webserver/Webserver.hpp"


#ifndef USING_CATCH
static bool validArguments(int argc, char *argv[])
{
	(void)argv;
	return argc == 2;
}

int main(int argc, char **argv)
{
	if (!validArguments(argc, argv))
	{
		PRINT_ERR << "Usage: [./webserv [CONFIGURATION_PATH]" << std::endl;
		return 1;
	}
	try
	{
		Config config_file(argv[1]);
		if (config_file.parser() == ERR)
		{
			return 1;
		}
		config_file.print();
		Webserver webserver(config_file.getAddressMap());
		if (webserver.init() == ERR)
		{
			return 1;
		}
		webserver.print();
		webserver.run();
	}
	catch(const std::exception& e)
	{
		PRINT_ERR << "main exception: " << e.what() << std::endl;
		return 1;
	}
	return 0;
}
#endif /* USING_CATCH */
