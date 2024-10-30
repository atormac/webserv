/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Socket.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lopoka <lopoka@student.hive.fi>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/28 12:28:21 by lopoka            #+#    #+#             */
/*   Updated: 2024/10/30 19:37:58 by user             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
#include <Socket.hpp>
#include <ServerConfig.hpp>
#include <unistd.h>


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
