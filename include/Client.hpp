#ifndef CLIENT_HPP
#define CLIENT_HPP

 #include <HttpServer.hpp>

class Request;

class Client
{
	public:
			int	fd;
			int	socket;

			time_t	start_time;
	
			bool is_cgi;
			int  *pipefd[2];

			std::string ip_addr;
			std::shared_ptr<Request> req;
			std::string response;

			Client();
			~Client();
			Client(int client_fd, int socket_fd, std::string ip);
};
#endif
