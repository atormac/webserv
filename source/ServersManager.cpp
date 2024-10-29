/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServersManager.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lopoka <lopoka@student.hive.fi>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/28 13:51:39 by lopoka            #+#    #+#             */
/*   Updated: 2024/10/29 14:02:19 by lopoka           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
#include <ServersManager.hpp>
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

bool ServersManager::_setNonblocking(int socketFd)
{
	int flags = fcntl(socketFd, F_GETFL, 0);
	if (fcntl(socketFd, F_SETFL, flags | O_NONBLOCK) == -1)
	{
		perror("fcntl");
		return false;
	}
	return true;
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
		
		_setNonblocking(socketFd);

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
			perror("bind AAAAA");
			//return false;
		}
		if (::listen(socketFd, LISTEN_BACKLOG) == -1)
		{
			perror("listen");
			//return false;
		}
		
		itr->second->setSocketDescriptor(socketFd);
		_socketFdToSockets.insert({socketFd, itr->second});
	}
}

bool ServersManager::listen()
{
	struct epoll_event events[MAX_EVENTS];

	_epoll_fd = epoll_create1(0);
	if (_epoll_fd == -1)
	{
		perror("epoll_create");
		return false;
	}

	for (std::map<int, Socket*>::iterator itr = _socketFdToSockets.begin(); itr != _socketFdToSockets.end(); itr++)
	{
		struct epoll_event ev;
		ev.events = EPOLLIN;
		ev.data.fd = itr->first; // socketFD

		if (::epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, itr->first, &ev) == -2) // itr->first == socketFd
		{
			perror("epoll_ctl");
			return false;
		}
	}
	
	while (true)
	{
		int nfds = ::epoll_wait(_epoll_fd, events, MAX_EVENTS, -1);

		if (nfds == -1)
			break;
		for (int i = 0; i < nfds; i++)
		{
			bool registered = false;
			epoll_event &event = events[i];
			for (std::map<int, Socket*>::iterator itr = _socketFdToSockets.begin(); itr != _socketFdToSockets.end(); itr++)
			{
				if (event.data.fd == itr->first) //Queue requests
				{
					registered = true;
					_socketAcceptClient(itr->first);
				}
			}
			if (registered)
				continue;
			if ((event.events & EPOLLIN) || (event.events & EPOLLOUT))
					_handleEvent(event);
		}
	}
	return true;
}

bool ServersManager::_socketAcceptClient(int socketFd)
{
	struct sockaddr_in peer_addr;
	socklen_t peer_addr_size = sizeof(peer_addr);
	struct epoll_event ev;

	int client_fd = accept(socketFd, (sockaddr *)&peer_addr, &peer_addr_size);
	if (client_fd == -1)
	{
		std::cerr << "accept() failed" << std::endl;
		return false;
	}
	_setNonblocking(client_fd);
	ev.events = EPOLLIN | EPOLLET;
	ev.data.fd = 0;
	ev.data.ptr = new Client(client_fd, inet_ntoa(peer_addr.sin_addr));
	if (epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, client_fd, &ev) == -1)
	{
		std::cerr << "epoll_ctl accept failed" << std::endl;
		return false;
	}
	return true;
}

bool ServersManager::_handleEvent(epoll_event &event)
{
	Client *client = (Client *)event.data.ptr;
	struct epoll_event ev_new;

	if (client == NULL)
		return false;
	if (event.events & EPOLLIN) //read
	{
		char buffer[2048 + 1] = { 0 };
		ssize_t bytes_read = read(client->fd, buffer, 2048);
		if (bytes_read < 0)
		{
			perror("read");
			return false;
		}
		std::cout << "[webserv] request received " << client->ip_addr
			  << std::endl;
		Request req;
		std::string req_str(buffer, bytes_read);
		req.parse(req_str);
		req.dump();
	
		//std::cout << buffer << std::endl;
		ev_new.events = EPOLLOUT | EPOLLET;
		ev_new.data.fd = 0;
		ev_new.data.ptr = event.data.ptr;
		if (epoll_ctl(this->_epoll_fd, EPOLL_CTL_MOD, client->fd, &ev_new) == -1)
		{
			perror("epoll_ctl");
			return false;
		}
	} else if (event.events & EPOLLOUT) //write
	{
		std::cout << "[webserv] write " << client->ip_addr << std::endl;
		_response(client->fd);
		if (epoll_ctl(this->_epoll_fd, EPOLL_CTL_DEL, client->fd, NULL) == -1)
		{
			perror("epoll_ctl");
			return false;
		}

		close(client->fd);
		delete client;
	}
	return true;
}

void ServersManager::_response(int client_fd)
{
	std::string html =
		"<!DOCTYPE html><html lang=\"en\"><body><h1>[WEBSERVER]</h1><p>HELLO WORLD</p></body></html>";
	std::ostringstream ss;
	ss << "HTTP/1.1 200 OK\nContent-Type: text/html\nContent-Length: " << html.size()
	   << "\n\n"
	   << html;
	std::string html_response = ss.str();

	write(client_fd, html_response.c_str(), html_response.size());
}

