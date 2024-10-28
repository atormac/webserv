#include "../include/Server.hpp"
#include "../include/Socket.hpp"
#include "../include/ServersManager.hpp"
#include <iostream>

int main(void)
{
	ServersManager sm;

	std::cout << "[webserv]" << std::endl;
	Server *server1 = new Server("127.0.0.1", 7051);
	Server *server2 = new Server("127.0.0.1", 7051);
	Server *server3 = new Server("127.0.0.1", 7051);
	Server *server4 = new Server("127.0.0.1", 7051);
	
	std::string port("127.0.0.1:7051");
	sm.addSocket(port, server1);
	sm.addSocket(port, server2);
	sm.addSocket(port, server3);
	sm.addSocket(port, server4);

	server1->init();
	server1->listen();
	return 0;
}
