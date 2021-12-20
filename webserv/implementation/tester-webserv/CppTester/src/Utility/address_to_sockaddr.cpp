#include "utility.hpp"
#include "DataStructures/Address.hpp"
#include "settings.hpp"
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <cstring>
#include <netdb.h>

namespace util {

static int resolveHostname(const std::string& hostname, struct sockaddr_in* dest) {
	struct addrinfo* result = nullptr;
	if (getaddrinfo(hostname.c_str(), NULL, NULL, &result) != 0) {
		return ERR;
	}

	struct addrinfo* it = result;
	while (it) {
		if (it->ai_family == AF_INET) {
			dest->sin_addr.s_addr = reinterpret_cast<sockaddr_in*> (it->ai_addr)->sin_addr.s_addr;
			break;
		}
		it = it->ai_next;
	}
	freeaddrinfo(result);
	return OK;
}

int addressToSockaddr(const Address& addr, struct sockaddr_in* dest) {
	memset(dest, 0, sizeof(sockaddr_in));
	int ret = inet_pton(AF_INET, addr.ip.c_str(), &(dest->sin_addr));
	if (ret == ERR) {
		return syscallError(_FUNC_ERR("inet_pton"));
	} else if (ret == 0) {
		if (resolveHostname(addr.ip, dest) == ERR) {
			return ERR;
		}
	}
	dest->sin_family = AF_INET;
	dest->sin_port = htons(addr.port);
	return OK;
}

}
