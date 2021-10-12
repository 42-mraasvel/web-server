#include <iostream>
#include <cstdio>
#include <sys/socket.h>
#include <cstdlib>
#include <netinet/in.h>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080

static void	exit_error(const std::string& message)
{
	std::cout << message << std::endl;
	exit (EXIT_FAILURE);
}

int	main()
{
	int	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0)
	{
		exit_error("Failed to create socket.");
	}

	struct sockaddr_in	server_address;
	memset(&server_address, '0', sizeof(server_address));
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(PORT);

	// Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, "127.0.0.1", &server_address.sin_addr) <= 0)
	{
		exit_error("Invalid address / Address not supported.");
	}

	if (connect(sock, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
	{
		exit_error("Connection Faield.");
	}

	std::string request =
"GET / HTTP/1.1
Host: localhost:8080
Upgrade-Insecure-Requests: 1
Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8
User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_14_6) AppleWebKit/605.1.15 (KHTML, like Gecko) Version/14.0.3 Safari/605.1.15
Accept-Language: en-us
Accept-Encoding: gzip, deflate
Connection: keep-alive

";
	send(sock, request.c_str(), request.size(), 0);
	std::cout << "Client sent request." << std::endl;

	// read from the connection
	char	buffer[30000] = {0};
	long	bytesRead = read(sock, buffer, 30000);
	std::cout << "Client received the message: " << buffer << std::endl;

	close (sock);
	return 0;
}
