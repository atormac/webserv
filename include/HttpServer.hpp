/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpServer.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lopoka <lopoka@student.hive.fi>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/28 11:17:19 by lopoka            #+#    #+#             */
/*   Updated: 2025/01/17 13:39:10 by atorma           ###   ########.fr       */
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
#include <set>


class HttpServer {
    private:
	int _epoll_fd;

	std::map<std::string, std::shared_ptr<Socket> > _portsToSockets;
	std::map<int, std::shared_ptr<Socket> > _socketFdToSockets;
	std::map<int, int> _socketToPort;

	std::unordered_map<int, std::shared_ptr<Client> > _clients;
	std::unordered_map<int, int> _cgi_to_client; //cgi fd -> client/conn fd

	void remove_fd(int fd);
	void timeout_clients(void);
	bool accept_client(int socket_fd);
	bool mod_fd(int fd, int ctl, int mask, std::shared_ptr<Client> cl);

	void handle_read(std::shared_ptr<Client> client);
	void handle_write(std::shared_ptr<Client> client);

	void finish_cgi_client(std::shared_ptr<Client> client);
	bool init_cgi_fds(std::shared_ptr<Client> current);

	void set_config(std::shared_ptr<Client> client, std::shared_ptr<Request> req);

    public:
	HttpServer();
	~HttpServer();

	void parseConfig(const std::string &filePath);
	std::vector<std::shared_ptr<ServerConfig> > matching_configs(int port);
	void close_server(void);
	void close_connections(void);
	bool init();
	void epoll(void);

	static void signal_handler(int code);
};

#endif
