#include "settings.hpp"
#include <cstdlib> // For exit() and EXIT_FAILURE
#include <iostream> // For cout
#include <poll.h>
#include <unistd.h>
#include "webserver/Webserver.hpp"
#include "color.hpp"
#include "config/Config.hpp"

#ifndef USING_CATCH
int main()
{
	Config config_file("file");
	config_file.print();
	Webserver webserver;
	webserver.init(config_file);
	webserver.print();
	webserver.run();
	return 0;
}
#endif /* USING_CATCH */
