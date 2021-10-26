# HTTP-Client

Simple C++ program to send one or multiple requests through a file or stdin to a server

# Sending a request

- Write an HTTP request in a regular file
- Run make
- Executable name: client.out
- Give filename(s) as arguments

# Stdin Examples

- cat [REQUESTS] | ./client.out -stdin
	This will send at most BUFFER_SIZE bytes to the server, not caring about the end of requests
	Recommended to use for testing byte by byte writing, or very large requests
	NOTE: this is only effective with HTTP/1.1 or connection: keep-alive header for HTTP/1.0
	because otherwise nginx will close the connection

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
In UserSettings.hpp you can modify the output (REQUEST and RESPONSE printing)
