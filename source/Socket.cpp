/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Socket.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lopoka <lopoka@student.hive.fi>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/28 12:28:21 by lopoka            #+#    #+#             */
/*   Updated: 2024/10/28 12:36:54 by lopoka           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
#include "../include/Socket.hpp"
#include "../include/HttpServer.hpp"

Socket::~Socket() {}

void Socket::addServer(HttpServer *server)
{
	_servers.push_back(server);
}
