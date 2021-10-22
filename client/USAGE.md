# HTTP-Client

Simple C++ program to send one or multiple requests through a file or stdin to a server

# Sending a request

- Write an HTTP request in a regular file
- Run make
- Executable name: client.out
- Give filename(s) as arguments

# Options
- -h [HOST_ADDR]	(DEFAULT = 127.0.0.1)
- -p [PORT_NUM]		(DEFAULT = 8080)
- -[ c | connection ] [ SINGLE | MULTIPLE ] (case insensitive) (DEFAULT = SINGLE)
- -stdin (read request from stdin instead, DEFAULT = false)
	NOTE: Does not work wth multiple connections
- -[ size | buffer_size ] [BUFFER_SIZE] (specify bytes per send) (DEFAULT = 4096)
- -timeout [TIMEOUT_SECONDS] (DEFAULT = 5), number of seconds client will wait for a response after writing on a single connection

# Usage
./client.out [OPTIONS] [REQUEST_FILES]
./run.sh [OPTIONS] [REQUEST_FILES]
*run.sh is a script that will automatically compile the project as well as execute it*

# Output
Currently it outputs the request and the file it's reading from.
By default it prints the header of the response, if you want to print the message body as well,
go to UserSettings.hpp and set the PRINT_MESSAGE_BODY define to non-zero
