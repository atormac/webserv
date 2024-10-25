#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <iostream>
#include <stdbool.h>
#include <stdlib.h>
#include <sstream>
#include <sys/epoll.h>
#include <signal.h>

#include "HttpServer.hpp"


int main(void)
{
	std::cout << "[webserv]" << std::endl;
	HttpServer server = HttpServer("127.0.0.1", 7051);
	server.init();
	server.listen();
	return 0;
}
