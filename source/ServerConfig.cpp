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
		size_t delim = line.find(" ");
		std::string element = (delim != std::string::npos) ? line.substr(0, delim) : line;
		std::string value = (delim != std::string::npos) ? WspcTrim(line.substr(delim + 1)) : "";
	
		if (line != "}" && element != "location" && element != "server_name" && element != "client_max_body_size")
			std::cout << "In serv block: |" << element << "| eq " << (element == "location") << std::endl;
		else if (element == "server_name")
		{
			_addName(value);
			std::cout << "Found server_name element: " << value << std::endl;
		}
		else if (element == "client_max_body_size")
		{
			_addMaxSize(value);
		}
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

// Setters
void ServerConfig::_addName(std::string &name)
{
	if (name.empty())
		throw std::runtime_error("_addName: Adding empty server name!");
	if (name.back() != ';')
		throw std::runtime_error("_addName: server_name line not terminated with semicolon!");
	name.pop_back();
	if (std::find(_names.begin(), _names.end(), name) != _names.end())
		throw std::runtime_error("_addName: Adding duplicate server name!");
	_names.push_back(name);
}

void ServerConfig::_addMaxSize(std::string &size)
{
	if (size.empty())
		throw std::runtime_error("_addMaxSize: Adding empty max size!");
	if (size.back() != ';')
		throw std::runtime_error("_addMaxSize: max size not terminated with semicolon!");
	size.pop_back();
	_maxSize = stot(size);
}

// Getters
std::vector<std::string> &ServerConfig::getNames() {return _names;}
size_t ServerConfig::getMaxSize() {return _maxSize;}
