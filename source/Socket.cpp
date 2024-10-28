/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Socket.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lopoka <lopoka@student.hive.fi>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/28 12:28:21 by lopoka            #+#    #+#             */
/*   Updated: 2024/10/28 15:13:06 by lopoka           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
#include "../include/Socket.hpp"
#include "../include/Server.hpp"

Socket::Socket(Server *server): _socketFd(-1), _servers{server} {}

Socket::~Socket()
{
	for (Server *i : _servers)
	{
		delete i;
		i = NULL;
	}
}

void Socket::addServer(Server *server)
{
	_servers.push_back(server);
}


const std::vector<Server *> Socket::getServers() const
{
	return _servers;
}
