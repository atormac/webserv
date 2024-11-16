/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpServer.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lopoka <lopoka@student.hive.fi>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/28 13:51:39 by lopoka            #+#    #+#             */
/*   Updated: 2024/11/16 21:47:47 by lopoka           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
#include <HttpServer.hpp>
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
#include <Utils.hpp>
#include <memory>

void HttpServer::parseConfig(const std::string &filePath)
{
	std::ifstream configFile;
	std::string	line;

	configFile.open(filePath.c_str());
	if (!configFile.is_open())
		throw std::runtime_error("ParseConfig: Coldn't open config file");
	std::cout << "Parsing config file" << std::endl;

	while (std::getline(configFile, line))
	{
		removeComments(line);
		if (line.empty())
			continue;
		if (line == "server")
		{
			std::shared_ptr<ServerConfig> server(new ServerConfig());
			server->parseServerConfig(configFile);
			
			// For testing
			for (std::string i: server->getNames())
				std::cout << "Server name: " << i << std::endl;
			std::cout << "Server max size: " << server->getMaxSize() << std::endl;
		}
		else
			throw std::runtime_error("ParseConfig: Unexpected value outside server block: " + line);
	}
	std::cout << "Config parsing completed" << std::endl;
}

HttpServer::~HttpServer() {}

bool HttpServer::_socketPresent(std::string &port)
{
	return _portsToSockets.find(port) != _portsToSockets.end() ? true : false;
}

void HttpServer::addSocket(std::string &port, ServerConfig *server)
{
	if (_socketPresent(port))
		_portsToSockets[port]->addServer(server);
	else
		_portsToSockets.insert({port, std::shared_ptr<Socket>(new Socket(server))});

	std::cout << "Servers in socket: " << _portsToSockets[port]->getServers().size() << std::endl;
}

bool HttpServer::set_nonblocking(int socketFd)
{
	int flags = 0;

	if (fcntl(socketFd, F_SETFL, flags | O_NONBLOCK) == -1)
	{
		perror("fcntl");
		return false;
	}
	return true;
}

void HttpServer::close_server(void)
{
	if (this->_epoll_fd == -1)
		return;
	for(const auto& e : this->_socketFdToSockets)
	{
		e.second->close_socket();
	}
	close(this->_epoll_fd);
	this->_epoll_fd = -1;
}

void HttpServer::init_sockets()
{
	for (std::map<std::string, std::shared_ptr<Socket>>::iterator itr = _portsToSockets.begin(); itr != _portsToSockets.end(); itr++)
	{
		std::string str = itr->first;
		size_t delim_pos = str.find(":");
		std::string ip = str.substr(0, delim_pos);
		delim_pos += 1;
		std::string s_port = str.substr(delim_pos, str.length() - delim_pos);
		int port = std::stoi(s_port);
		
		int socketFd = bind_socket(ip, port);

		itr->second->setSocketDescriptor(socketFd);
		_socketFdToSockets.insert({socketFd, itr->second});
	}
}

int HttpServer::bind_socket(std::string ip, int port)
{
	struct sockaddr_in socket_addr;
	int opt = 1;

	int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_fd == -1)
	{
		perror("socket");
		return -1;
	}
	
	set_nonblocking(socket_fd);
	if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
	{
		perror("setsockopt");
		return -1;
	}
	socket_addr.sin_family = AF_INET;
	socket_addr.sin_port = htons(port);
	socket_addr.sin_addr.s_addr = inet_addr(ip.c_str());

	if (bind(socket_fd, (sockaddr *)&socket_addr, sizeof(socket_addr)) == -1)
	{
		perror("bind");
		return -1;
	}
	if (::listen(socket_fd, LISTEN_BACKLOG) == -1)
	{
		perror("listen");
		return -1;
	}
	std::cout << "Listening on: " << ip << ":" << port << std::endl;
	return socket_fd;
}


bool HttpServer::listen()
{
	struct epoll_event events[MAX_EVENTS];

	_epoll_fd = epoll_create1(0);
	if (_epoll_fd == -1)
	{
		perror("epoll_create");
		return false;
	}

	for(const auto& so : this->_socketFdToSockets)
	{
		struct epoll_event ev;
		ev.events = EPOLLIN;
		ev.data.fd = so.first; //socket_fd

		if (::epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, so.first, &ev) == -2)
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
			epoll_event &e = events[i];

			if ((e.events & EPOLLERR) || (e.events & EPOLLRDHUP) || (e.events & EPOLLHUP))
			{
				remove_client((Client *)e.data.ptr);
				continue;
			}
			if (this->_socketFdToSockets.count(e.data.fd))
			{
				accept_client(e.data.fd);
				continue;
			}
			if (e.events & EPOLLIN)
				handle_read(e);
			else if (e.events & EPOLLOUT)
				handle_write(e);
		}
	}
	return true;
}

bool HttpServer::accept_client(int _socket_fd)
{
	struct sockaddr_in peer_addr;
	socklen_t peer_addr_size = sizeof(peer_addr);
	struct epoll_event ev;

	int client_fd = accept(_socket_fd, (sockaddr *)&peer_addr, &peer_addr_size);
	if (client_fd == -1)
	{
		perror("accept()");
		return false;
	}
	if (!set_nonblocking(client_fd))
	{
		perror("set_nonblocking");
		close(client_fd);
		return false;
	}
	ev.events = EPOLLIN | EPOLLET;
	ev.data.fd = 0;
	ev.data.ptr = new Client(client_fd, inet_ntoa(peer_addr.sin_addr));

	Client *client = (Client *)ev.data.ptr;
	//client->req = new Request();

	if (epoll_ctl(this->_epoll_fd, EPOLL_CTL_ADD, client_fd, &ev) == -1)
	{
		std::cerr << "epoll_ctl accept failed" << std::endl;
		remove_client((Client *)ev.data.ptr);
		return false;
	}
	_clients.emplace(client_fd, client);
	std::cout << "client added\n";
	return true;
}

void HttpServer::remove_client(Client *client)
{
	if (!client)
		return;
	if (epoll_ctl(this->_epoll_fd, EPOLL_CTL_DEL, client->fd, NULL) == -1)
	{
		perror("remove_client epoll_ctl");
		return;
	}
	close(client->fd);
	_clients.erase(client->fd);
}


#define READ_BUFFER_SIZE 1024

void HttpServer::handle_read(epoll_event &event)
{
	Client *client = (Client *)event.data.ptr;
	struct epoll_event ev_new;
	char buffer[READ_BUFFER_SIZE];

	if (client == NULL)
		return;

	ssize_t bytes_read = read(client->fd, buffer, READ_BUFFER_SIZE);
	if (bytes_read == -1)
	{
		remove_client(client);
	}

	State state = client->req->parse(buffer, bytes_read);

	ev_new.events = EPOLLET | EPOLLIN;
	ev_new.data.fd = 0;
	ev_new.data.ptr = event.data.ptr;
	if (state == State::Complete || state == State::Error || bytes_read == 0)
	{
		ev_new.events = EPOLLET | EPOLLOUT;
		client->req->dump();
	}
	if (epoll_ctl(this->_epoll_fd, EPOLL_CTL_MOD, client->fd, &ev_new) == -1)
	{
		perror("epoll_ctl");
		remove_client(client);
	}
}

void HttpServer::handle_write(epoll_event &event)
{
	Client *client = (Client *)event.data.ptr;
	struct epoll_event ev_new;

	if (client == NULL)
		return;
	std::cout << "[webserv] write " << client->ip_addr << std::endl;
	if (client->response.size() == 0)
	{
		Response resp(client->req);
		client->response = resp.buffer.str();
	}

	ssize_t resp_size = client->response.size();
	ssize_t bytes_written = write(client->fd, client->response.data(), client->response.size());
	if (bytes_written <= 0)
	{
		remove_client(client);
		return;
	}
	if (bytes_written < resp_size)	
	{
		std::cout << "writing chunk\n";	
		client->response.erase(0, bytes_written);
		ev_new.events = EPOLLET | EPOLLOUT;
		ev_new.data.fd = 0;
		ev_new.data.ptr = event.data.ptr;
		if (epoll_ctl(this->_epoll_fd, EPOLL_CTL_MOD, client->fd, &ev_new) == -1)
		{
			perror("epoll_ctl");
		}
		return;
	}
	remove_client(client);
}
