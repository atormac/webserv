#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <Request.hpp>
#include <unordered_map>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstring>
#include <Io.hpp>


class Response
{
	private:
		int	_http_code;
		void	build_response(int status);
		void	build_response(Request *req, int status);

		void	handle_post(Request *req);
		void	handle_delete(Request *req);
		void	handle_get(Request *req);

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
