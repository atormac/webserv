#include <ServerConfig.hpp>

#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <iostream>
#include <stdlib.h>
#include <sstream>
#include <sys/epoll.h>
#include <signal.h>
#include <fstream>
#include <Utils.hpp>


int signo = 0;

void signal_handler(int code)
{
	if (code == SIGINT)
		signo = code;
}

ServerConfig::ServerConfig()
{
	signal(SIGINT, ::signal_handler);
}

ServerConfig::~ServerConfig()
{
	std::cout << "ServerConfig deconstructor called" << std::endl;
}

void ServerConfig::parse(std::ifstream &configFile)
{
	std::string line;

	skipEmptyLines(configFile, line);
	if (!configFile)
		throw std::runtime_error("parseServerConfig: Empty server block!");
	if (line != "{")
		throw std::runtime_error("parseServerConfig: No '{' opening server block!");

	while (skipEmptyLines(configFile, line), configFile)
	{
		if (line != "}")
			std::cout << "In serv block: " << line << std::endl;
		else if (line == "}")
			break;
		else
			throw std::runtime_error("parseServerConfig: Unknown element in server block: " + line);
	}
	if (line != "}")
		throw std::runtime_error("parseSerever: Unterminated server block!");
}
