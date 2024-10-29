#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <iostream>
#include <sys/epoll.h>

class Client
{
	private:
	public:
		int	fd;
		std::string ip_addr;
		Client(int client_fd, std::string ip);
		~Client();
};
#endif
