#include "utility/utility.hpp"
#include <string>

namespace WebservUtility {

bool stringEndsWith(const std::string &s, const std::string& to_find,
				std::size_t begin, std::size_t end) {
	if (end == std::string::npos) {
		end = s.size();
	}

	return (end - begin) >= to_find.size()
		&& s.compare(end - to_find.size(), to_find.size(), to_find) == 0;
}

}
