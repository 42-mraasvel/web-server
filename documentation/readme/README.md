# HTTP Web Server

A Hypertext Transfer Protocol (HTTP) 1.1 web server written in C++ 98, inspired by NginX.

This is a project for Codam Coding College.
It can be used to serve any type of content to clients through the HTTP protocol.
Content such as files, for websites or other purposes.

## Supported Functionalities

The server listens on a socket (IP:Port).

It will accept connections on that socket and process HTTP requests.

- HTTP Methods:

	- GET

		Fetch resources from the origin server.

	- POST

		Upload or update resources on the origin server.

	- DELETE

		Delete resources on the origin server.

- Common Gateway Interface (CGI)

	CGI allows the user to specify an external program to execute specific requests. The web server converts the HTTP request to a CGI request, reads the CGI response and converts it back into an HTTP response, which is returned to the client.

- Configuration Options

	Implemented similarly to NginX's configuration. All directives are explained in detail [here](#configuration-syntax-and-directives).

## Usage

### 1. Write a configuration file

A configuration file can optionally be given to the web server as argument. See the link above for syntax and all configuration options.

If not specified, a default configuration is used instead.

The default configuration can be modified in the run.sh script.

### 2. Build and run the web server

	./run.sh [OUTPUT_OPTIONS] [ConfigurationFile]

	OUTPUT_OPTIONS = "verbose" | "printall" | "default"

	verbose prints extra information on requests and responses.
	printall prints extra debug information on top of verbose.
	default resets output options to the minimum.

The web server is now able to accept requests, this can be done through the browser or any other valid HTTP client.

### 3. Send requests

A typical HTTP request is sent through either a browser or other client programs.

With the default configuration, you can test the web server by going to the following link:

>http://localhost:8080

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

	location = /exact_match/ {
		root /var/www/other;
	}
}
```
- Server Block

	```
	server {
		...
	}
	```

	Server blocks contain options for how the web server handles requests. There can be multiple server blocks.

	Server blocks are resolved using the request address, the top-most block is the default.

- Listen Directive
	```
	listen 127.0.0.1:8080;
	```
	Instructs the server to accept requests on this IP:Port pair, multiple listen directives can be present in the same block.
	```
	listen 8081;
	```
	If an IP is not specified, the server will listen to all addresses for this port.

- Server Names (optional)
	```
	server_name example.com *.example.com www.example.* "";
	```
	Used to further resolve server blocks for a request if there are multiple matches using the `Host` header field in the request.

	Server names support wildcards `*`.

	The empty server name `""` will match with an empty `Host` header field. If no server name is specified, `""` will be set as the default value.

- Error Pages (optional)
	```
	error_page 404 /error_pages/404.html;
	```

	Specify a custom file to respond to a given error code. If the error_page directive is not specified or the given file doesn't exist, the web server's default response is used.

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

	Location blocks contain more specific options for how the web server further handles requests, the request target is used to resolve the location block.

	The most specific match is used, unless the `=` (exact match) option is specified, in which case the request target has to match exactly.

	Examples:

	`/outdated/index.html` matches to `/outdated`

	`/exact_match/` matches to `/exact_match/`

	`/exact_match/index.html` matches to `/` because it's the most specific match

- Allowed Methods (optional)

	```
	allowed_methods GET POST DELETE;
	```

	Specifies the accepted HTTP methods for a particular location, other methods will be rejected.

- Root Directory (optional)
	```
	root /var/www/html;
	```

	Specifies the directory in which the web server looks for files to serve. If no root directive is specified, the default value is `/var/www/html`.

- Auto Index (optional)
	```
	autoindex on;
	autoindex off;
	```

	Auto index enables the web server to return a directory listing, when the request target is a directory and a default index is not found.

	Default value: on

- Index (optional)
	```
	index index.html index.php;
	```
	Specifies the default files to search for if the request target is a directory. The search stops at the first match.

- CGI (optional)
	```
	cgi .php /usr/bin/php-cgi;
	cgi .py /usr/bin/python3;
	```

	If the request target ends in the given file extension, the executable given will be executed as a CGI program with the request target's full path as an argument.

- Upload Store (optional)
	```
	upload_store /var/www/upload;
	```

	The directory specified by the upload_store directive is used for POST requests to upload files. When not specified, files will be stored in the location block's root directory.

- Redirections (optional)
	```
	return 301 http://example.com:8080;
	```

	If a return directive is specified, the client is redirected to the given address.
