/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpServer.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lopoka <lopoka@student.hive.fi>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/28 11:17:19 by lopoka            #+#    #+#             */
/*   Updated: 2024/11/23 18:03:08 by user             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
#ifndef HTTPSERVER_HPP
#define HTTPSERVER_HPP

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
			
			//rename for sanity
			void	remove_client(Client *client);
			bool	accept_client(int socket_fd);
			void	handle_read(epoll_event &event);
			void	handle_write(epoll_event &event);
			void	find_config(epoll_event &event);

	public:
			~HttpServer();			

			void parseConfig(const std::string &filePath);
			void close_server(void);
			bool init();
			void epoll();
};

#endif
