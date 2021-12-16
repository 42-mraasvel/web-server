#include "settings.hpp"
#include <cstdlib> // For exit() and EXIT_FAILURE
#include <iostream> // For cout
#include <poll.h>
#include <unistd.h>
#include "webserver/Webserver.hpp"
#include "color.hpp"
#include "config/Config.hpp"
#include "utility/SmartPointer.hpp"
#include "handler/RequestHandler.hpp"
#include "tmp/create_address_map.hpp"

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
			std::cout << RED_BOLD "Warning: No configuration file given, using default config" << RESET_COLOR << std::endl;
			configuration = "src/src_normal/config/resources/default.conf";
		}
		else
		{
			std::cerr << RED_BOLD "Invalid argument amount" << std::endl;
			exit(1);
		}
		Config config_file(configuration);

		if (config_file.parser() == ERR)
		{
			std::cout << "PARSING ERROR EXIT PROGRAM" << std::endl;
			exit(1);
		}
		config_file.print();
		Webserver webserver(config_file.getAddressMap());
		if (webserver.init())
			return (1);
		webserver.print();
		webserver.run();
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
	}
	return 0;
}
#endif /* USING_CATCH */
