/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Socket.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lopoka <lopoka@student.hive.fi>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/28 12:28:21 by lopoka            #+#    #+#             */
/*   Updated: 2024/10/28 14:24:19 by lopoka           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
#include "../include/Socket.hpp"
#include "../include/HttpServer.hpp"

Socket::Socket(HttpServer *server): _socketFd(-1), _servers{server} {}

Socket::~Socket()
{
	for (HttpServer *i : _servers)
	{
		delete i;
		i = NULL;
	}
}

void Socket::addServer(HttpServer *server)
{
	_servers.push_back(server);
}


const std::vector<HttpServer *> Socket::getServers() const
{
	return _servers;
}
