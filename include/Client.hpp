#ifndef CLIENT_HPP
#define CLIENT_HPP

 #include <HttpServer.hpp>

class Request;
class Cgi;

enum 
{
	CONN_REGULAR,
	CONN_WAIT_CGI,
	CONN_CGI,
};

class Client
{
	public:

			int	conn_type;
			int	fd;
			int	socket;

			time_t	start_time;
	

			Client	*ref;
			int	cgi_to[2];
			int	cgi_from[2];

			int	pid;

			std::string ip_addr;

			std::shared_ptr<Request> req;
			std::shared_ptr<Response> resp;

			std::string response;

			Client();
			~Client();
			Client(int client_fd, int socket_fd, std::string ip);
			Client(int client_fd, int pid, Client *ref);
};
#endif
