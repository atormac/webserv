#ifndef DEFINES_HPP
#define DEFINES_HPP

enum
{
	STATUS_OK = 200,
	STATUS_CREATED = 201,
	STATUS_BAD_REQUEST = 400,
	STATUS_FORBIDDEN = 403,
	STATUS_NOT_FOUND = 404,
	STATUS_METHOD_NOT_ALLOWED = 405,
	STATUS_TOO_LARGE = 413,
};

enum
{
	METHOD_GET,
	METHOD_POST,
	METHOD_DELETE
};

#define READ_BUFFER_SIZE 1024

#define SERVER_NAME "webserv"
#define REQUEST_BODY_LIMIT 10 * 1024 * 1024 //10MB
#define CRLF "\r\n"

#define DEFAULT_404 "<!DOCTYPE html> <html><head><title>404 Not Found</title></head><body><h1>404 Not Found</h1></body></html>"

#endif
