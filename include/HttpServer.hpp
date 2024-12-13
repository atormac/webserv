/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpServer.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lopoka <lopoka@student.hive.fi>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/28 11:17:19 by lopoka            #+#    #+#             */
/*   Updated: 2024/12/13 17:11:11 by atorma           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
#ifndef HTTPSERVER_HPP
#define HTTPSERVER_HPP

class Socket;
class Client;
class Request;
class Response;

#include <StandardHeaders.hpp>
#include <ServerConfig.hpp>
#include <Location.hpp>
#include <Response.hpp>
#include <Defines.hpp>
#include <Request.hpp>
#include <Client.hpp>
#include <Socket.hpp>
#include <Utils.hpp>
#include <Str.hpp>
#include <Cgi.hpp>
#include <Io.hpp>

class HttpServer
{
	private:
			int	_epoll_fd;
			int	_client_count;

			std::map<std::string, std::shared_ptr<Socket>> _portsToSockets;
			std::map<int, std::shared_ptr<Socket>> _socketFdToSockets;
			std::map<int, std::shared_ptr<Client>> _clients;
			
			void	remove_client(int fd);
			void	cull_clients(void);
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
			void epoll(void);
};

#endif
