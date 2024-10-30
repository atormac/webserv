#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <iostream>
#include <Request.hpp>
#include <fstream>
#include <sstream>
#include <string>
#include <cstring>

enum
{
	STATUS_OK = 200,
	STATUS_NOT_FOUND = 404,
};
//std::unordered_map<std::string, MimeType> MimeMap = {{".html",  "text/html"},
class Response
{
	private:
		int	_http_status;
		void	build_response(int status);
		std::string status_message(int status);
		bool	read_www_file(std::string filename);
	public:
		int status;
		std::ostringstream _body;
		std::ostringstream buffer;

		Response(Request *req);
		~Response();
};
#endif
