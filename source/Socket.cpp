/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Socket.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lopoka <lopoka@student.hive.fi>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/28 12:28:21 by lopoka            #+#    #+#             */
/*   Updated: 2024/12/28 19:00:21 by user             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
#include <HttpServer.hpp>

Socket::Socket(std::shared_ptr<ServerConfig> server)
	: _socketFd(-1)
	, _servers{ server }
{
}

Socket::~Socket()
{
	close_socket();
}

void Socket::close_socket(void)
{
	if (this->_socketFd >= 0)
	{
		close(this->_socketFd);
		this->_socketFd = -1;
	}
}

void Socket::addServer(std::shared_ptr<ServerConfig> server)
{
	_servers.push_back(server);
}

const std::vector<std::shared_ptr<ServerConfig> > Socket::getServers() const
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
