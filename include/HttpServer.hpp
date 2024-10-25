#ifndef HTTPSERVER_HPP
#define HTTPSERVER_HPP

#include <iostream>
#include <sys/epoll.h>

#define MAX_EVENTS 10
#define LISTEN_BACKLOG 32

class HttpServer
{
	private:
		int _socket_fd;
		int _port;
		std::string _ip;
		struct epoll_event ev[MAX_EVENTS];
		struct epoll_event events[MAX_EVENTS];
	public:
		HttpServer(std::string ip, int port);
		~HttpServer();
		bool	init(void);
		bool	listen(void);
		void	response(int client_fd);
};
#endif
