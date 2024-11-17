/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Socket.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lopoka <lopoka@student.hive.fi>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/28 12:28:21 by lopoka            #+#    #+#             */
/*   Updated: 2024/11/17 16:09:14 by user             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
#include <Socket.hpp>
#include <ServerConfig.hpp>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>


Socket::Socket(ServerConfig *server): _socketFd(-1), _servers{server} {}

Socket::~Socket()
{
	(void)_socketFd;
	for (ServerConfig *i : _servers)
	{
		delete i;
		i = NULL;
	}
}

void	Socket::close_socket(void)
{
	close(this->_socketFd);
	std::cout << "socket: " << _socketFd << " closed" << std::endl;
}
void Socket::addServer(ServerConfig *server)
{
	_servers.push_back(server);
}


const std::vector<ServerConfig *> Socket::getServers() const
{
	return _servers;
}


void Socket::setSocketDescriptor(int socketFd)
{
	_socketFd = socketFd;
}

int Socket::bind_socket(std::string ip, int port)
{
	struct sockaddr_in socket_addr;
	int opt = 1;

	int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_fd == -1)
	{
		perror("socket");
		return -1;
	}
	
	if (!Io::set_nonblocking(socket_fd))
	{
		perror("fcntl");
		return -1;
	}
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
