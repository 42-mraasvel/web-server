#pragma once

// #define CRLF "\r\n"
#define DELIMITERS "(),/:;<=>?@[\\]{}\""
#define WHITESPACE " \t"
#define SEPERATORS DELIMITERS WHITESPACE
#define SUB_DELIMS "!$&'()*+,;="
#define LOWERCASE "abcdefghijklmnopqrstuvwxyz"
#define UPPERCASE "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
#define ALPHA LOWERCASE UPPERCASE
#define DIGIT "0123456789"
#define ALPHA_NUMERIC ALPHA DIGIT
#define HEXDIG DIGIT "abcdefABCDEF"
#define UNRESERVED ALPHA DIGIT "-._~"
#define PCHAR UNRESERVED SUB_DELIMS ":@%"
