#ifndef CGI_HPP
#define CGI_HPP

#include <unordered_map>
#include <iostream>
#include <memory>
#include <Defines.hpp>
#include <Request.hpp>

extern std::unordered_map<std::string, std::string> cgi_map;

class Cgi
{
	private:
		std::string _cgi;
		std::string _cgi_arg;
		std::vector<std::string> _env;

		void close_pipes(int *fd);
		bool find_cgi(std::string uri);
		void env_set(const std::string &key, const std::string &value);
		void env_set_vars(std::shared_ptr<Request> request);
	public:
		Cgi();
		~Cgi();
		bool execute(std::shared_ptr<Request> request, std::string &body);
};
#endif
