#include "utility/utility.hpp"
#include <string>

#define BASE_STR "0123456789abcdefghijklmnopqrstuvwxyz"
#define BASE_STR_UPCASE "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"

namespace WebservUtility
{

	namespace _detail_ {
		void itoaGetDigits(unsigned long n, unsigned int base, std::string& result) {
			if (n >= base) {
				itoaGetDigits(n / static_cast<unsigned long>(base), base, result);
			}

			char c = BASE_STR [(n % base)];
			result.push_back(c);
		}

		std::string itoaInternal(unsigned long n, unsigned int base, bool negative = false) {
			if (base < 2 || base > 36) {
				return "";
			}

			std::string result;
			if (negative) {
				result.push_back('-');
			}

			itoaGetDigits(n, base, result);
			return result;
		}
	}

std::string itoa(unsigned long n, unsigned int base) {
	return _detail_::itoaInternal(n, base);
}

std::string itoa(unsigned int n, unsigned int base) {
	return itoa(static_cast<unsigned long>(n), base);
}

std::string itoa(long n, unsigned int base) {
	if (n < 0) {
		return _detail_::itoaInternal(static_cast<unsigned long> (-n), base, true);
	}

	return _detail_::itoaInternal(static_cast<unsigned long> (n), base);
}

std::string itoa(int n, unsigned int base) {
	return itoa(static_cast<long>(n), base);
}

}
