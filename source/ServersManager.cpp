/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServersManager.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lopoka <lopoka@student.hive.fi>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/28 13:51:39 by lopoka            #+#    #+#             */
/*   Updated: 2024/10/28 19:49:15 by lopoka           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
#include <ServersManager.hpp>

ServersManager::~ServersManager()
{
	for (std::map<std::string, Socket*>::iterator itr = _portsToSockets.begin();
		itr != _portsToSockets.end(); itr++)
	{
		delete itr->second;
	}
}

bool ServersManager::_socketPresent(std::string &port)
{
	return _portsToSockets.find(port) != _portsToSockets.end() ? true : false;
}

void ServersManager::addSocket(std::string &port, Server *server)
{
	if (_socketPresent(port))
		_portsToSockets[port]->addServer(server);
	else
		_portsToSockets.insert({port, new Socket(server)});

	std::cout << "Servers in socket: " << _portsToSockets[port]->getServers().size() << std::endl;
}


void ServersManager::_initSockets()
{
	for (std::map<std::string, Socket*>::iterator itr = _portsToSockets.begin(); itr != _portsToSockets.end(); itr++)
	{
		struct sockaddr_in socket_addr;
		int opt = 1;

		int socketFd = socket(AF_INET, SOCK_STREAM, 0);
		if (socketFd == -1)
		{
			perror("socket");
			//return false; We could throw errors
		}
		if (setsockopt(socketFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
		{
			perror("setsockopt");
			//return false;
		}

		std::string str = itr->first;
		size_t delim_pos = str.find(":");
		std::string ip = str.substr(0, delim_pos);
		delim_pos += 1;
		std::string s_port = str.substr(delim_pos, str.length() - delim_pos);
		int port = boost::lexical_cast<int>(s_port);
		
		socket_addr.sin_family = AF_INET;
		socket_addr.sin_port = htons(port);
		socket_addr.sin_addr.s_addr = inet_addr(ip.c_str());

		if (bind(socketFd, (sockaddr *)&socket_addr, sizeof(socket_addr)) == -1)
		{
			perror("bind");
			//return false;
		}
		itr->second->setSocketDescriptor(socketFd);
		_socketFdToSockets.insert({socketFd, itr->second});
	}
}


void ServersManager::listen()
{
	struct epoll_event events[MAX_EVENTS];

	/*if (::listen(this->_socket_fd, LISTEN_BACKLOG) == -1)
	{
		perror("listen");
		return false;
	}
	std::cout << "[webserv] server listening on port: " << this->_port << std::endl;*/

	_epoll_fd = epoll_create1(0);
	if (_epoll_fd == -1)
	{
		perror("epoll_create");
		//return false;
	}

	for (std::map<int, Socket*>::iterator itr = _socketFdToSockets.begin(); itr != _socketFdToSockets.end(); itr++)
	{
		struct epoll_event ev;
		ev.events = EPOLLIN;
		ev.data.fd = itr->first; // socketFD

		if (::epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, itr->first, &ev) == -2) // itr->first == socketFd
		{
			perror("epoll_ctl");
			//return false;
		}
	}
	
	while (true)
	{
		int nfds = ::epoll_wait(_epoll_fd, events, MAX_EVENTS, -1);

		if (nfds == -1)
			break;
		for (int i = 0; i < nfds; i++)
		{
			epoll_event &event = events[i];
			for (std::map<int, Socket*>::iterator itr = _socketFdToSockets.begin(); itr != _socketFdToSockets.end(); itr++)
			{
				if (event.data.fd == itr->first) //Queue requests
					itr->second->getServers()[0]->accept_client(); // for now the first server in the sockt accepts, later on we can find correct host
				else if ((event.events & EPOLLIN) || (event.events & EPOLLOUT))
					itr->second->getServers()[0]->handle_event(event);
			}
		}
	}
	//return true;
}
