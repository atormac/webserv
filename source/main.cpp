#include "../include/HttpServer.hpp"
#include "../include/Socket.hpp"
#include "../include/ServersManager.hpp"
#include <iostream>

int main(void)
{
	ServersManager sm;

	std::cout << "[webserv]" << std::endl;
	HttpServer server = HttpServer("127.0.0.1", 7051);
	server.init();
	server.listen();
	return 0;
}
