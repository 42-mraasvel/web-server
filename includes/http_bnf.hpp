#pragma once

// #define CRLF "\r\n"
#define SEPERATORS "()<>@,;:\\\"/[]?={} \t"
#define SUB_DELIMS "!$&'()*+,;="
#define ALPHA "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"
#define DIGIT "0123456789"
#define HEXDIG DIGIT "abcdefABCDEF"
#define UNRESERVED ALPHA DIGIT "-._~"
#define PCHAR UNRESERVED SUB_DELIMS ":@%"
