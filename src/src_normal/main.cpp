#include "settings.hpp"
#include "utility/Output.hpp"
#include "webserver/Webserver.hpp"

#ifndef USING_CATCH
int main(int argc, char **argv)
{
	std::string configuration;
	try
	{
		if (argc == 2)
		{
			configuration = argv[1];
		}
		else if (argc == 1)
		{
			PRINT_WARNING << "No configuration file given, using default config" << std::endl;
			configuration = "src/src_normal/config/resources/default.conf";
		}
		else
		{
			PRINT_ERR << "invalid argument amount" << std::endl;
			return 1;
		}
		Config config_file(configuration);
		if (config_file.parser() == ERR)
		{
			PRINT_ERR << "PARSING ERROR EXIT PROGRAM" << std::endl;
			return 1;
		}
		config_file.print();
		Webserver webserver(config_file.getAddressMap());
		if (webserver.init())
		{
			return 1;
		}
		webserver.print();
		webserver.run();
	}
	catch(const std::exception& e)
	{
		PRINT_ERR << e.what() << std::endl;
	}
	return 0;
}
#endif /* USING_CATCH */
