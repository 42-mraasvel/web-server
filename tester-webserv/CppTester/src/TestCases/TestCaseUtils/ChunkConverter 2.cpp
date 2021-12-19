#include "TestCaseUtil.hpp"
#include "Utility/utility.hpp"
#include "settings.hpp"
#include <string>

namespace util {

static std::size_t appendChunkString(std::string& dest, const std::string& src, std::size_t pos, std::size_t chunk_size) {
	chunk_size = std::min(chunk_size, src.size() - pos);
	dest.append(util::itoa(chunk_size, 16));
	dest.append(CRLF);
	dest.append(src, pos, chunk_size);
	dest.append(CRLF);
	return pos + chunk_size;
}

std::string convertToChunked(const std::string& str, std::size_t chunk_size) {
	std::string result;
	std::size_t index = 0;
	while (index != str.size()) {
		index = appendChunkString(result, str, index, chunk_size);
	}
	result.append("0" CRLF CRLF);
	return result;
}

}
