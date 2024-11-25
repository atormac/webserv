#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <Request.hpp>
#include <unordered_map>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstring>
#include <memory>
#include <Io.hpp>
#include <Cgi.hpp>
#include <ServerConfig.hpp>

class Location;

class Response
{
	private:
		int	_status_code;
		std::shared_ptr<Request> req;

		void	build_response(int status);

		void	handle_post(void);
		void	handle_delete(void);
		void	handle_get(void);

		//Location find_location(void);
		std::shared_ptr <Location> find_location(void);

		bool	directory_index(std::string path);
		std::string date_now(void);
		std::string get_content_type(std::string uri);
		bool is_cgi(std::string uri);
		void do_cgi(void);
	public:
		int status;
		std::ostringstream _body;
		std::ostringstream buffer;

		Response(std::shared_ptr<Request> req);
		~Response();
};
#endif
