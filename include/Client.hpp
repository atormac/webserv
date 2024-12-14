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
			int	pipefd[2];

			std::string ip_addr;
			std::shared_ptr<Request> req;
			std::string response;

			Client();
			~Client();
			Client(int client_fd, int socket_fd, std::string ip);
};
#endif
