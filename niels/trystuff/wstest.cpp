#include "WebservTest.hpp"

void    err_n_die(const char *fmt, ...)
{
    int errno_save;
    va_list ap;

    errno_save = errno;
    va_start(ap, fmt);
	vfprintf(stdout, fmt, ap);
	fprintf(stdout,"\n");
	fflush(stdout);
	if (errno_save != 0)
	{
		fprintf(stdout, "(errno = %d) : %s\n",errno_save, strerror(errno_save));
		fprintf(stdout,"\n");
		fflush(stdout);
	}
	va_end(ap);
	exit(1);
}


char *bin2hex(const unsigned char *input, size_t len)
{
	char *result;
	char hexits[17] = "0123456789ABCDEF";
	if (input == NULL || len <= 0)
		return (NULL);
	int resultlength = (len*3)+1;

	result = (char *)malloc(resultlength);
	bzero(result, resultlength);

	for (int i = 0; i < len; i++)
	{
		result[i*3] = hexits[input[i] >> 4];
		result[(i*3)+1] = hexits[input[i] & 0X0F];
		result[(i*3)+2] = ' ';
	}
	return (result);
}


int main(int argc, char **argv)
{
	int                 listenfd, connfd, n;
	struct sockaddr_in  serveraddr;
	uint8_t             buff[MAXLINE + 1];
	uint8_t             recvline[MAXLINE+1];

	if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		err_n_die("socket error.");

	bzero(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family		= AF_INET;
	serveraddr.sin_addr.s_addr	= htonl(INADDR_ANY);
	serveraddr.sin_port			= htons(SERVER_PORT);

	if ((bind(listenfd, (SA *) &serveraddr, sizeof(serveraddr))) < 0)
		err_n_die("bind error.");
	if ((listen(listenfd, 10)) < 0)
		err_n_die("listen error.");

	for( ; ; )
	{
		struct sockaddr_in addr;
		socklen_t addr_len;
		printf("waiting for a connection on port %d\n", SERVER_PORT);
		fflush(stdout);
		connfd = accept(listenfd, (SA *) NULL, NULL);
		memset(recvline, 0, MAXLINE);
		while ( (n = read(connfd, recvline, MAXLINE) ) > 0)
		{
			fprintf(stdout, "\n%s\n\n%s", bin2hex(recvline, n), recvline);
			if (recvline[n-1] == '\n')
				break;
			memset(recvline, 0, MAXLINE);
		}
		if (n < 0)
			err_n_die("read error");
		snprintf((char*)buff, sizeof(buff), "HTTP/1.0 200 OK\r\n\r\nHELLO");

		write(connfd, (char*)buff, strlen((char *)buff));
		close(connfd);
	}

}





