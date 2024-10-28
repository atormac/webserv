#include "../include/HttpServer.hpp"
#include "../include/Socket.hpp"
#include "../include/ServersManager.hpp"
#include <iostream>

int main(void)
{
	ServersManager sm;

	std::cout << "[webserv]" << std::endl;
	HttpServer *server1 = new HttpServer("127.0.0.1", 7051);
	HttpServer *server2 = new HttpServer("127.0.0.1", 7051);
	HttpServer *server3 = new HttpServer("127.0.0.1", 7051);
	HttpServer *server4 = new HttpServer("127.0.0.1", 7051);
	
	std::string port("127.0.0.1:7051");
	sm.addSocket(port, server1);
	sm.addSocket(port, server2);
	sm.addSocket(port, server3);
	sm.addSocket(port, server4);

	server1->init();
	server1->listen();
	return 0;
}
