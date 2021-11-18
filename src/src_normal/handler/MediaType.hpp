#pragma once
#include "parser/HeaderField.hpp"

namespace MediaType
{

typedef HeaderField Map;
void	initMap(Map & map);
std::string	getMediaType(std::string const & file);

}
