#pragma once
#define OK 0
#define ERR -1

#define BACKLOG 50
#define POLL_TIMEOUT 5000 // 5 seconds
#define TIMEOUT 60 // 60 second timeout for client and CGI
#define CRLF "\r\n"
#define NEWLINE CRLF
#define EOHEADER CRLF CRLF

/*
Config Specific Defines
*/
#define DEFAULT_PORT 80
#define DEFAULT_ROOT "/var/www/html"

/*
Parsing Specific Defines
*/
#define MAX_HEADER_SIZE 8192

/*
Response Specific Defines
*/
#define FD_TABLE_MAX_SIZE 100
#define BUFFER_SIZE 4096
#define CHUNK_THRESHOLD 2048
#define CHUNK_TAIL "0" CRLF CRLF
#define RETRY_AFTER_SECONDS 30
#define CLOSE_CONNECTION_DELAY 0
#define RESPONSE_QUEUE_MAX_SIZE 50 // Number of responses the connection can execute concurrently
#define REQUEST_QUEUE_THRESHOLD 50 // Number of requests in the queue that will prompt the connection to stop reading in more

/*
Printing
*/
#define PRINT_DEBUG_FLAG true
#define PRINT_INFO_FLAG true
#define PRINT_ERR_FLAG true


/*
Very unnecessary header includes
*/

#include <cstdio> // RM
#include <iostream> // RM
#include "color.hpp"
#include "utility/macros.hpp"
#include "outputstream/Output.hpp"
#include <cassert> // RM
