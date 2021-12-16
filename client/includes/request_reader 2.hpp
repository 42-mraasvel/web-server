#pragma once

#include "settings.hpp"

/*
Reads from stdin or file depending on settings
returns data read
*/

class RequestReader {
public:
	RequestReader(Settings* setings);

private:
	Settings* settings;
};
