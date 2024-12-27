/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpServer.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lopoka <lopoka@student.hive.fi>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/28 11:17:19 by lopoka            #+#    #+#             */
/*   Updated: 2024/12/27 22:29:48 by user             ###   ########.fr       */
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

			//std::vector <pair(int, std::shared_ptr<Client> _clients;
			std::map<int, std::shared_ptr<Client>> _clients;
			std::map<int, std::shared_ptr<Client>> _cgi_to_client;

			std::deque<std::pair<int, time_t>> _deque;

			void	remove_fd(int fd);
			void	cull_clients(void);
			bool	insert_map(int const& k, std::shared_ptr <Client> const& v);
			bool	accept_client(int socket_fd);
			bool	add_fd(int fd, int ctl, int mask, std::shared_ptr<Client> cl);

			void 	handle_read(std::shared_ptr <Client> client);
			void 	handle_write(std::shared_ptr <Client> client);

			void	finish_cgi_client(std::shared_ptr <Client> client);
			void	add_cgi_fds(std::shared_ptr <Client> current);


			void	find_config(epoll_event &event);
			void 	set_config(std::shared_ptr <Client> client, std::shared_ptr <Request> req);

	public:
			HttpServer();			
			~HttpServer();			

			void parseConfig(const std::string &filePath);
			void close_server(void);
			bool init();
			void epoll(void);

			static void signal_handler(int code);
};

#endif
