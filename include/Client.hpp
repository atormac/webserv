#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <iostream>
#include <sys/epoll.h>
#include <Request.hpp>

class Client
{
	private:
	public:
		int	fd;
		std::string ip_addr;
		Request *req;
		std::string response;
		Client(int client_fd, std::string ip);
		~Client();
};
#endif
