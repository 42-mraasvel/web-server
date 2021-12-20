#include "utility.hpp"

namespace util
{

/* Check if x + y overflows */
bool additionOverflow(std::size_t x, std::size_t y)
{
	return x + y < x;
}

}
