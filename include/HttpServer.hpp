/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpServer.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lopoka <lopoka@student.hive.fi>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/28 11:17:19 by lopoka            #+#    #+#             */
/*   Updated: 2024/10/30 19:58:32 by user             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
#pragma once
#include <map>
#include <ServerConfig.hpp>
#include <Socket.hpp>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <boost/lexical_cast.hpp>
#include <fcntl.h>
#include <unistd.h>

class Socket;

class HttpServer
{
	private:
			int	_epoll_fd;
			std::map<std::string, Socket *> _portsToSockets;
			std::map<int, Socket *> _socketFdToSockets;
			
			bool _socketPresent(std::string &port);
			//rename for sanity
			bool	set_nonblocking(int socketFd);
			void	remove_client(Client *client);
			bool	accept_client(int socket_fd);
			bool	handle_event(epoll_event &event);

			void _response(int client_fd);
	public:
			~HttpServer();			

			void parseConfig(const std::string &filePath);
			void close_server(void);
			void addSocket(std::string &port, ServerConfig *server);
			void initSockets();
			bool listen();
};
