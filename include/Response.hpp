#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <Request.hpp>
#include <unordered_map>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstring>


class Response
{
	private:
		int	_http_code;
		void	build_response(int status);
		void	build_response(Request *req, int status);
		void	get_resource(Request *req);
		bool	read_www_file(std::string filename);
		void	directory_index(Request *req, std::string path);
		std::string date_now(void);
		std::string get_content_type(std::string uri);
	public:
		int status;
		std::ostringstream _body;
		std::ostringstream buffer;

		Response(Request *req);
		~Response();
};
#endif
