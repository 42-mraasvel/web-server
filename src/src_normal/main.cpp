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
int main()
{
	Config config_file("src/src_normal/config/resources/default.conf");
	// config_file.print();
	Webserver webserver(config_file.getAddressMap());
	if (webserver.init(config_file))
		return (1);
	webserver.print();
	webserver.run();
	return 0;
}
#endif /* USING_CATCH */
