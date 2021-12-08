# Webserver

A Hypertext Transfer Protocol (HTTP) 1.1 webserver written in C++ 98, inspired by NginX.
This is a project for Codam Coding College.
It can be used to serve any type of content to clients through the HTTP protocol.
Content such as files, for websites or any other purpose.

## Supported Functionalities

The server can listen on any socket (IP:PORT), it will accept connections on that socket and process HTTP requests from that connection.

- HTTP Methods:

	- GET

		Fetch resources from the origin server

	- POST

		Upload or update resources in the origin server

	- DELETE

		Delete resources in the origin server

- Common Gateway Interface (CGI)

	An interface that allows the user to specify an external program to execute specific requests. The webserver converts the HTTP request to a CGI request, reads the CGI response and converts it back into an HTTP response, which is returned to the client.

- Configuration Options

	Implemented similarly to NginX's configuration. All directives are explained in detail here: 
	
	TODO: LINK

## Usage

### 1. Writing a configuration file

A configuration file can optionally be given to the webserver as argument. See above for syntax and all configuration options.

If not specified, a default configuration is used instead.

### 2. Building and running the webserve

	./run.sh [OPTIONS] [ConfigurationFile]

The webserver is now able to accept requests, this can be done through the browser or any other valid HTTP client.

TODO: link to options and describe what the options do, etc

### 3. Sending Requests

A typical HTTP request is done through either a browser, or a client program. You can use either a browser, curl, python, or any other custom implementation.

If you're using the default configuration, you can use the following link:

>http://localhost:8080

### Appendix

The `localhost` specifies the IP address of the server. It can also be an exact IP or a domain name that maps to an IP, such as `127.0.0.1` or `example.com`

The `:8080` portion specifies the port you want to send the request to.

## Configuration Syntax and Directives

Configuration file example:

```
server {
	listen 127.0.0.1:8080;
	listen 8081;
	server_name example.com *.example.com www.example.* "";
	
	error_page 404 /error_pages/404.html;
	client_body_size 100M;

	location / {
		allowed_methods GET POST DELETE;

		root /var/www/html;
		autoindex on;
		index index.html index.php;

		cgi .php /usr/bin/php-cgi;
		cgi .py /usr/bin/python3;

		upload_store /var/www/upload;
	}

	location /outdated {
		return 301 http://example.com:8080;
	}
}
```
- Server Block

	```
	server {
		...
	}
	```

	These contain the options for how the webserver should handle the requests.

	The server blocks are resolved using the request interface address, the top-most block is the default server.

	There can be multiple server blocks, each block can listen on multiple IP:PORT pairs.

- Listen Directive
	```
	listen 127.0.0.1:8080;
	```
	Instructs the server to accept requests on this IP:PORT pair, multiple listen directives can be present in the same block.
	```
	listen 8081;
	```
	If an IP is not specified, the server will bind to all interfaces for this port.

- Server Names (optional)
	```
	server_name example.com *.example.com www.example.* "";
	```
	Used to further resolve server block for a request if there are multiple matches using the `Host` header field in the request.

	The wildcard `*` matches any number of characters before or after the given string.

	The empty server_name `""` will match with an empty Host header field. If no server_names are specified, this is the default value.

- Error Pages (optional)
	```
	error_page 404 /error_pages/404.html;
	```

	Specify a custom file to respond to certain error codes. If not specified or the file doesn't exist, the webserver's default response is used.

- Client Max Body Size (optional)
	```
	client_body_size 100M;
	```

	Specifies maximum size of a request's payload body. If it exceeds this size, it will be rejected.

	Value syntax:

	```
	Bytes: 10000
	Kilbytes, Megabytes, Gigabytes: 1K 1M 1G
	```

- Location Blocks

	```
	location / {
		...
	}

	location /outdated {
		...
	}

	location = /exact_match/ {
		...
	}
	```

	More specific options for how the webserver handles the request further, the request target is used to resolve the location block.

	The most specific match is used, unless the `=` (exact match) option is specified, in which case the request target has to match exactly.

	Examples:

	`/outdated/index.html` matches to `/outdated`

	`/exact_match/` matches to `/exact_match/`

	`/exact_match/index.html` matches to `/` because it's the most specific match

- Allowed Methods (optional)

	```
	allowed_methods GET POST DELETE;
	```

	Specifies the accepted HTTP methods for a particular location, all other methods will be rejected.

- Root Directory (optional)
	```
	root /var/www/html;
	```

	Specifies the directory in which the webserver should look for files to serve. The default value is `/var/www/html`

- Auto Index (optional)
	```
	autoindex on;
	autoindex off;
	```

	If enabled and the request target is a directory, and a default file cannot be found, a directory listing is returned.

	Default value: on

- Index (optional)
	```
	index index.html index.php;
	```
	Specifies the default files to search for if the request target is a directory. Stops at the first match.

- CGI (optional)
	```
	cgi .php /usr/bin/php-cgi;
	cgi .py /usr/bin/python3;
	```

	If the request target ends in the given file extension, the executable given will be executed as a CGI program, with the request target's full path as argument.

- Uploading Files (optional)
	```
	upload_store /var/www/upload;
	```

	For POST requests, if an upload_store is specified, that directory will be where the file is stored instead of the server root.

- Redirections (optional)
	```
	return 301 http://example.com:8080;
	```

	If a return directive is specified, the client is redirected to the given address.
