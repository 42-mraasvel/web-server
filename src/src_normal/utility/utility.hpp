#pragma once

#include <vector>
#include <string>

namespace WebservUtility
{

std::vector<std::string> splitString(	const std::string& input,
										char delim);
std::vector<std::string> splitString(	const std::string& input,
										const std::string& delim_set);

}
