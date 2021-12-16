#include "settings.hpp"
#include <cstdlib> // For exit() and EXIT_FAILURE
#include <poll.h>
#include <unistd.h>
#include "webserver/Webserver.hpp"
#include "color.hpp"
#include "config/Config.hpp"
#include "utility/SmartPointer.hpp"
#include "handler/RequestHandler.hpp"
#include "tmp/create_address_map.hpp"
#include "outputstream/Output.hpp"

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
			PRINT << RED_BOLD "Warning: " RESET_COLOR "No configuration file given, using default config" << std::endl;
			configuration = "src/src_normal/config/resources/default.conf";
		}
		else
		{
			PRINT_ERR << "invalid argument amount" << std::endl;
			exit(1);
		}
		Config config_file(configuration);
		config_file.print();
		Webserver webserver(config_file.getAddressMap());
		if (webserver.init() == ERR)
			return (1);
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
