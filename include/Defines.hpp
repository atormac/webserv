#ifndef DEFINES_HPP
#define DEFINES_HPP

#include <HttpServer.hpp>

enum {
	STATUS_OK = 200,
	STATUS_CREATED = 201,
	STATUS_BAD_REQUEST = 400,
	STATUS_FORBIDDEN = 403,
	STATUS_NOT_FOUND = 404,
	STATUS_METHOD_NOT_ALLOWED = 405,
	STATUS_TOO_LARGE = 413,
	STATUS_INTERNAL_ERROR= 500,
};

enum { METHOD_GET, METHOD_POST, METHOD_DELETE };

#define TIMEOUT_SECONDS (1 * 60)
#define CGI_TIMEOUT (5 * 60)

#define READ 0
#define WRITE 1

#define READ_BUFFER_SIZE 2048

#define SERVER_NAME "webserv"
#define REQUEST_BODY_LIMIT 10 * 1024 * 1024 //10MB
#define CRLF "\r\n"

extern std::unordered_map<int, std::string> code_map;
extern std::unordered_map<std::string, std::string> mime_map;

#endif
