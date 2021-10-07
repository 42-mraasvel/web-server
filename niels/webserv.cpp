#include "webserv.hpp"

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













int main(int argc, char **argv)
{
	int             	listenfd;
	int             	conffd;
	int             	n;
	uint8_t         	buff[MAXLINE + 1];
	uint8_t         	recvline[MAXLINE + 1];
	struct sockaddr_in	serveraddr;

	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	if (listenfd < 0)
		err_n_die("socket error.");






	
}

