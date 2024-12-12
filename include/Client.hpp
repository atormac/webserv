#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <iostream>
#include <sys/epoll.h>
#include <Request.hpp>
#include <memory>

class Client
{
	private:
	public:
		int	fd;
		int	socket;
		std::string ip_addr;
		//Request *req;
		std::shared_ptr<Request> req;
		std::string response;
		//test
		std::string _setCookies;

		Client();
		~Client();
		Client(int client_fd, int socket_fd, std::string ip);
};
#endif
