/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServersManager.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lopoka <lopoka@student.hive.fi>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/28 11:17:19 by lopoka            #+#    #+#             */
/*   Updated: 2024/10/28 17:46:20 by lopoka           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
#pragma once
#include <map>
#include <Server.hpp>
#include <Socket.hpp>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <boost/lexical_cast.hpp>

class Socket;

class ServersManager
{
	private:
			std::map<std::string, Socket *> _portsToSockets;
			std::map<int, Socket *> _socketFdToSockets;
			
			bool _socketPresent(std::string &port);
			void _initSockets();

	public:
			~ServersManager();			

			void addSocket(std::string &port, Server *server);
};
