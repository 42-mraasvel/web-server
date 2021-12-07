#pragma once
#define OK 0
#define ERR -1

#define BACKLOG 10 //TODO: check later
#define POLL_TIMEOUT 10000 // 10 seconds
#define TIMEOUT 60 // 60 second timeout for now
#define CRLF "\r\n"
#define NEWLINE CRLF
#define EOHEADER CRLF CRLF

/*
Config Specific Defines
*/
#define DEFAULT_PORT 80

/*
Parsing Specific Defines
*/
#define MAX_HEADER_SIZE 8192 // TODO: evaluate [RFC7230] : Section 3.1.1. (page 22)

/*
Response Specific Defines
*/
#define FD_TABLE_MAX_SIZE 50 // TODO: evaluate later
#define BUFFER_SIZE 4096 // TODO: evaluate later
#define CHUNK_THRESHOLD 2048 // TODO: evaluate later
#define CHUNK_TAIL "0" CRLF CRLF
#define RETRY_AFTER_SECONDS 30
#define CLOSE_CONNECTION_DELAY 5 // TODO: evaluate later
#define RESPONSE_QUEUE_MAX_SIZE 50 // Number of responses the connection can execute concurrently
#define REQUEST_QUEUE_THRESHOLD 50 // Number of requests in the queue that will prompt the connection to stop reading in more

/*
Printing
*/
#define PRINT_ERR true


/*
Very unnecessary header includes
*/

#include <cstdio> // RM
#include <iostream> // RM
#include "color.hpp"
#include "utility/macros.hpp"
#include <cassert> // RM
