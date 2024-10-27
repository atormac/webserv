#ifndef HTTPSERVER_HPP
#define HTTPSERVER_HPP

#include "../include/Client.hpp"
#include <iostream>
#include <sys/epoll.h>

#define MAX_EVENTS 10
#define LISTEN_BACKLOG 32

void	signal_handler(int code);
class HttpServer
{
	private:
		int _epoll_fd;
		int _socket_fd;
		int _port;
		std::string _ip;
	public:
		HttpServer(std::string ip, int port);
		~HttpServer();
		void	signal_handler(int code);
		bool	set_nonblocking(int fd);
		bool	accept_client(void);
		bool	handle_event(epoll_event &event);
		bool	init(void);
		bool	listen(void);
		void	response(int client_fd);
};
#endif
