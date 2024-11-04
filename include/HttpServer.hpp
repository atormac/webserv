/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpServer.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lopoka <lopoka@student.hive.fi>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/28 11:17:19 by lopoka            #+#    #+#             */
/*   Updated: 2024/11/04 16:24:42 by user             ###   ########.fr       */
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
			std::map<std::string, std::shared_ptr<Socket>> _portsToSockets;
			std::map<int, std::shared_ptr<Socket>> _socketFdToSockets;
			std::map<int, std::shared_ptr<Client>> _clients;
			
			bool _socketPresent(std::string &port);
			//rename for sanity
			bool	set_nonblocking(int socketFd);
			int	bind_socket(std::string ip, int port);
			void	remove_client(Client *client);
			bool	accept_client(int socket_fd);
			void	handle_read(epoll_event &event);
			void	handle_write(epoll_event &event);

	public:
			~HttpServer();			

			void parseConfig(const std::string &filePath);
			void close_server(void);
			void addSocket(std::string &port, ServerConfig *server);
			void init_sockets();
			bool listen();
};
