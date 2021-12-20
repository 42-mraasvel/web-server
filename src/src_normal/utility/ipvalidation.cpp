#include "utility.hpp"
#include "parser/ParserUtils.hpp"
#include <string>

namespace WebservUtility
{

bool validIpv4(std::string const & ip) {
	std::size_t index = 0;
	std::size_t dott_count = 0;
	while (index < ip.size()) {
		if (isDigit(ip[index])) {
			std::size_t start = index;
			skip(ip, index, isDigit);
			if (index - start > 3 || strtol(ip.substr(start, index - start)) > 255) {
				return false;
			}
		} else if (ip[index] == '.') {
			dott_count += 1;
			index++;
		} else {
			return false;
		}
	}
	return dott_count == 3 && isDigit(ip[ip.size() - 1]);
}

}
