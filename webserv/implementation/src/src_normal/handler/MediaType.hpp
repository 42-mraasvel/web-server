#pragma once
#include "parser/HeaderField.hpp"

namespace MediaType
{

typedef HeaderField Map;
Map	initMap();
std::string	getMediaType(std::string const & file);

}
