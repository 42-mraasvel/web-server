#pragma once
#include "utility.hpp"


#define __STRINGIZE(x) #x
#define __STR(x) __STRINGIZE(x)
// Gives string version of "FILENAME:LINENUMBER FUNCTION_NAME: PARAMETER"
#define _FUNC_ERR(x) \
(std::string(__FILE__) + ":" + util::itoa(__LINE__) + " " + std::string(__func__) + ": " x)
