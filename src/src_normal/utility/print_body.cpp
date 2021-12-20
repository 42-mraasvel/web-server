#include "outputstream/Output.hpp"
#include <string>

namespace WebservUtility
{

static void printBytes(std::string const & message_body)
{
	const std::string exceptions = "\r\n";
	for (size_t i = 0; i < message_body.size(); i++) {
		if (isprint(message_body[i]) || exceptions.find(message_body[i]) != std::string::npos) {
			PRINT_INFO << message_body[i];
		} else {
			PRINT_INFO << static_cast<int>(message_body[i]);
		}
	}
}

void printBody(std::string const & message_body)
{
	if (message_body.size() <= 1000)
	{
		printBytes(message_body);
	}
	else
	{
		PRINT_INFO << "Body too large to print" << std::endl;
	}

}

}
