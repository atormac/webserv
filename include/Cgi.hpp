#ifndef CGI_HPP
#define CGI_HPP

#include <unordered_map>
#include <iostream>
#include <memory>
#include <Defines.hpp>
#include <Request.hpp>

extern std::unordered_map<std::string, std::string> cgi_map;

class Location;

class Cgi
{
	private:
		std::string _interpreter;
		std::string _script_path;
		std::vector<std::string> _env;
		std::vector<int> _pids;

		void close_pipes(int *fd);
		void env_set(const std::string &key, const std::string &value);
		void env_set_vars(std::shared_ptr<Request> request);
		bool parent_process(int pid, int *fd, std::string &body);
		void child_process(int *fd, std::vector <char *> args);
	public:
		Cgi();
		Cgi(std::shared_ptr <Location> location, std::shared_ptr<Request> request);
	
		~Cgi();
	
		bool execute(std::string &body);
		static bool is_cgi(std::shared_ptr <Location> location, std::string uri);
};
#endif
