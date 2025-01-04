#ifndef CGI_HPP
#define CGI_HPP

#include <HttpServer.hpp>

extern std::unordered_map<std::string, std::string> cgi_map;

class Location;

class Cgi
{
	private:
			std::string _interpreter;
			std::string _script_path;
			std::string _script_dir;
			std::vector<std::string> _env;

			void env_set(const std::string &key, const std::string &value);
			void env_set_vars(std::shared_ptr<Request> request);
	
			bool parent_init(int pid, int *fd_from, int *fd_to);
			void child_process(std::shared_ptr <Client> client, std::vector <char *> args, int *fd_from, int *fd_to);
	public:
			Cgi();
			Cgi(std::shared_ptr <Location> location, std::shared_ptr<Request> request);
			~Cgi();
		
			bool start(std::shared_ptr <Client> client);
			static void wait_kill(int pid);
			static bool finish(int pid, int *fd_from, int *fd_to);
			static void close_pipes(int *fd);
			static bool is_cgi(std::shared_ptr <Location> location, std::string uri);
};
#endif
