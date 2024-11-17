#include <Socket.hpp>
#include <HttpServer.hpp>
#include <ServerConfig.hpp>
#include <iostream>
#include <Str.hpp>
#include <regex>


void parseConfig(const std::string &filePath);

int main(void)
{
	HttpServer server;

	ServerConfig *cfg1 = new ServerConfig();
	ServerConfig *cfg2 = new ServerConfig();
	ServerConfig *cfg3 = new ServerConfig();
	
	std::string port1("127.0.0.1:7051");
	std::string port2("127.0.0.1:7052");
	std::string port3("127.0.0.1:7053");

	server.addSocket(port1, cfg1);
	server.addSocket(port2, cfg2);
	server.addSocket(port3, cfg3);

	server.init();
	server.epoll();
	server.close_server();

	/*
	HttpServer sm;

	sm.parseConfig("test.conf");
	*/

	return 0;
}
