#include <Server.hpp>
#include <Socket.hpp>
#include <ServersManager.hpp>
#include <iostream>

int main(void)
{
	std::cout << "[webserv]" << std::endl;
	Server *server = new Server();
	server->add("127.0.0.1", 7051);
	server->add("127.0.0.1", 7052);
	server->add("127.0.0.1", 7053);
	server->epoll();
	server->close_server();
	delete server;
	return 0;
}
