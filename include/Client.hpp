#ifndef CLIENT_HPP
#define CLIENT_HPP

 #include <HttpServer.hpp>

class Request;
class Cgi;

enum 
{
	CL_NORMAL,
	CL_CGI_INIT,
	CL_CGI_WRITE,
	CL_CGI_READ,
};

class Client
{
	public:
			Client	*old;

			int	fd;
			int	socket;

			time_t	start_time;
	
			int	status;
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
};
#endif
