/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServersManager.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lopoka <lopoka@student.hive.fi>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/28 11:17:19 by lopoka            #+#    #+#             */
/*   Updated: 2024/10/30 15:45:20 by lopoka           ###   ########.fr       */
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
			int	_epoll_fd;
			std::map<std::string, Socket *> _portsToSockets;
			std::map<int, Socket *> _socketFdToSockets;
			
			bool _socketPresent(std::string &port);
			bool _setNonblocking(int socketFd);
			//void _initSockets();
			bool _socketAcceptClient(int socketFd);
			bool _handleEvent(epoll_event &event);
			void _response(int client_fd);
	public:
			~ServersManager();			

			void parseConfig(const std::string &filePath);
			void addSocket(std::string &port, Server *server);
			void _initSockets();
			bool listen();
};
