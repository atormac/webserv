/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpServer.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lopoka <lopoka@student.hive.fi>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/28 11:17:19 by lopoka            #+#    #+#             */
/*   Updated: 2024/12/10 15:13:10 by atorma           ###   ########.fr       */
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
//#include <boost/lexical_cast.hpp>
#include <fcntl.h>
#include <unistd.h>

class Socket;

class HttpServer
{
	private:
			int	_epoll_fd;
			int	_client_count;

			std::map<std::string, std::shared_ptr<Socket>> _portsToSockets;
			std::map<int, std::shared_ptr<Socket>> _socketFdToSockets;
			std::map<int, std::shared_ptr<Client>> _clients;
			
			//rename for sanity
			void	remove_client(Client *client);
			bool	accept_client(int socket_fd);
			void	handle_read(epoll_event &event);
			void	handle_write(epoll_event &event);
			void	find_config(epoll_event &event);
			void	set_config(Client *client, std::shared_ptr <Request> req);

	public:
			~HttpServer();			

			void parseConfig(const std::string &filePath);
			void close_server(void);
			bool init();
			void epoll();
};

#endif
