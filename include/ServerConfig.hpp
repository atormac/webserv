#ifndef SERVERCONFIG_HPP
#define SERVERCONFIG_HPP

#include <Client.hpp>
#include <iostream>
#include <sys/epoll.h>
#include <vector>
#include <Request.hpp>
#include <Response.hpp>
#include <algorithm>
#include <Location.hpp>
#include <memory>

#define MAX_EVENTS 10
#define LISTEN_BACKLOG 32

void	signal_handler(int code);

class Location;

class ServerConfig
{
	private:
			std::vector<std::string> _names;
			size_t _maxSize;
			std::map<unsigned int, std::string> _errorPages;
			std::vector<std::shared_ptr<Location>> _locations;
		
			void _addName(std::stringstream &ss);
			void _addMaxSize(std::stringstream &ss);
			void _addErrorPage(std::stringstream &ss);
			void _addLocation(std::shared_ptr<Location> location);

	public:
			ServerConfig();
			~ServerConfig();

			void parseServerConfig(std::ifstream &configFile);

			// Getters
			std::vector<std::string> &getNames();
			size_t getMaxSize();
			std::vector<std::shared_ptr<Location>> &getLocations();
};
#endif
