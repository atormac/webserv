#ifndef CGI_HPP
#define CGI_HPP

#include <Request.hpp>
#include <unordered_map>
#include <iostream>
#include <memory>

extern std::unordered_map<std::string, std::string> cgi_map;

class Cgi
{
	private:
		std::string _cgi;
		std::string _cgi_arg;
		std::vector<std::string> _env;

		void close_pipes(int *fd);
		bool find_cgi(std::string uri);
		void env_set(std::string key, std::string value);
		void env_set_vars(std::shared_ptr<Request> request);
	public:
		Cgi();
		~Cgi();
		bool execute(std::shared_ptr<Request> request, std::string &body);
};
#endif
