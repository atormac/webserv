#ifndef HTTPSERVER_HPP
#define HTTPSERVER_HPP

#include <Client.hpp>
#include <iostream>
#include <sys/epoll.h>
#include <vector>
#include <Request.hpp>

#define MAX_EVENTS 10
#define LISTEN_BACKLOG 32

void	signal_handler(int code);

struct server_entry
{
	int socket_fd;
	int port;
	std::string ip;
};

class Server
{
	private:
		int _epoll_fd;
		std::vector<server_entry> _entries;
	public:
		Server();
		~Server();
		Server(std::string ip, int port);
		void	signal_handler(int code);
		void	close_server(void);
		bool	set_nonblocking(int fd);
		bool	accept_client(int _socket_fd);
		bool	handle_event(epoll_event &event);
		bool	add(std::string ip, int port);
		bool	epoll(void);
		void	response(int client_fd);
		void	parseServer(std::ifstream &configFile);
};
#endif
