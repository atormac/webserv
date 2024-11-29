#include <Socket.hpp>
#include <HttpServer.hpp>
#include <ServerConfig.hpp>
#include <iostream>
#include <Str.hpp>
#include <regex>


void parseConfig(const std::string &filePath);

int main(int argc, char **argv)
{
	if (argc != 2)
	{
		std::cerr << "Usage: ./webserv <config file>" << std::endl;
		return EXIT_FAILURE;
	}
	HttpServer server;

	try
	{
		server.parseConfig(argv[1]);
	}
	catch(const std::exception& e)
	{
		std::cerr << "Config parsing failed: " << e.what() << std::endl;
		return 1;
	}

	server.init();
	server.epoll();
	server.close_server();

	return 0;
}
