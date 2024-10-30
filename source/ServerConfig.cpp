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
		throw std::runtime_error("parseServer: Empty server block!");
	if (line != "{")
		throw std::runtime_error("parseServer: No '{' opening server block!");

	while (skipEmptyLines(configFile, line), configFile)
	{
		line = WspcTrim(line);
		std::string element = line.substr(0, line.find(" "));
	
		if (line != "}" && element != "location" && element != "server_name")
			std::cout << "In serv block: |" << element << "| eq " << (element=="location") << std::endl;
		else if (element == "server_name")
			std::cout << "Found server_name element" << std::endl;
		else if (element == "location")
		{
			while (std::getline(configFile, line), line.find("}") == std::string::npos);
		}
		else if (line == "}")
			break;
		else
			throw std::runtime_error("parseServer: Unknown element in server block: " + line);	
	}
	//std::cout << "Line: |" << line << "|" << std::endl;
	if (line != "}")
		throw std::runtime_error("parseSerever: Unterminated server block!");
}
