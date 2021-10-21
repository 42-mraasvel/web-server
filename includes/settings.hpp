#pragma once
#define OK 0
#define ERR -1
#define INCOMPLETE 1

#define BACKLOG 10 //TODO: check later
#define TIMEOUT 10000
#define BUFFER_SIZE 4096 // TODO: evaluate later
#define CRLF "\r\n"
#define NEWLINE CRLF
#define EOHEADER CRLF CRLF

/*
Parsing Specific Defines
*/

#define MAX_HEADER_SIZE 8192 // TODO: evaluate [RFC7230] : Section 3.1.1. (page 22)

/*
Very unnecessary header includes
*/

#include <cstdio> // RM
#include <iostream> // RM
#include "color.hpp"
#include <cassert> // RM
