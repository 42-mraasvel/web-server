#ifndef WEBSERVTEST_HPP
#define WEBSERV

#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <string>
#include <cstdio>
#include <limits>
#include <map>
#include <algorithm>
#include <vector>
#include <set>
#include <utility>
#include <list>
#include <iterator>
#include <sys/socket.h> 
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>


#define SERVER_PORT 4242

#define MAXLINE 4096
#define SA struct sockaddr


void    err_n_die(const char *fmt, ...);
char *bin2hex(const unsigned char *input, size_t len);




#endif