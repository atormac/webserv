#ifndef SERVERCONFIG_HPP
#define SERVERCONFIG_HPP

#include <Client.hpp>
#include <iostream>
#include <sys/epoll.h>
#include <vector>
#include <Request.hpp>
#include <Response.hpp>

#define MAX_EVENTS 10
#define LISTEN_BACKLOG 32

void	signal_handler(int code);

struct server_entry
{
	int socket_fd;
	int port;
	std::string ip;
};

class ServerConfig
{
	private:
		int _epoll_fd;
		std::vector<server_entry> _entries;
		void remove_client(Client *client);
	public:
		ServerConfig();
		~ServerConfig();
		ServerConfig(std::string ip, int port);
		void	parse(std::ifstream &configFile);
};
#endif
