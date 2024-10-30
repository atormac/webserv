#include <Server.hpp>
#include <Socket.hpp>
#include <ServersManager.hpp>
#include <iostream>


void parseConfig(const std::string &filePath);

int main(void)
{
	/*std::cout << "[webserv]" << std::endl;
	Server *server = new Server();
	server->add("127.0.0.1", 7051);
	server->add("127.0.0.1", 7052);
	server->add("127.0.0.1", 7053);
	server->epoll();
	server->close_server();
	delete server;*/

	/*
	ServersManager sm;

	Server *server1 = new Server();
	//server1->add("127.0.0.1", 7051);
	
	Server *server2 = new Server();
	//server2->add("127.0.0.1", 7052);
	
	Server *server3 = new Server();
	//server3->add("127.0.0.1", 7053);
	
	Server *server4 = new Server();
	//server4->add("127.0.0.1", 7054);

	std::string port1("127.0.0.1:7051");
	sm.addSocket(port1, server1);
	std::string port2("127.0.0.1:7052");
	sm.addSocket(port2, server2);
	std::string port3("127.0.0.1:7053");
	sm.addSocket(port3, server3);
	std::string port4("127.0.0.1:7054");
	sm.addSocket(port4, server4);

	sm._initSockets();
	sm.listen();*/

	ServersManager sm;

	sm.parseConfig("test.conf");

	return 0;
}
