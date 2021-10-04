#include "settings.hpp"
#include <cstdlib> // For exit() and EXIT_FAILURE
#include <iostream> // For cout
#include <poll.h>
#include <unistd.h>
#include "webserver/Webserver.hpp"
#include "color.hpp"
#include "config/Config.hpp"

int main()
{
	Config config_file("file");
	Webserver webserver;
	webserver.initWebserver(config_file);
	return 0;
}
