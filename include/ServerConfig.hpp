#ifndef SERVERCONFIG_HPP
#define SERVERCONFIG_HPP

#include <Client.hpp>
#include <iostream>
#include <sys/epoll.h>
#include <vector>
#include <Request.hpp>
#include <Response.hpp>
#include <algorithm>

#define MAX_EVENTS 10
#define LISTEN_BACKLOG 32

void	signal_handler(int code);

class ServerConfig
{
	private:
			std::vector<std::string> _names;
			size_t _maxSize;
		
			void _addName(std::string &name);
			void _addMaxSize(std::string &size);

	public:
			ServerConfig();
			~ServerConfig();

			void parse(std::ifstream &configFile);

			std::vector<std::string> &getNames();
			size_t getMaxSize();
};
#endif
