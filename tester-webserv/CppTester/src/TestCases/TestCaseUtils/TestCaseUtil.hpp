#pragma once

#include <string>

namespace util {

std::string convertToChunked(const std::string& str, std::size_t chunk_size = 1024);

}
