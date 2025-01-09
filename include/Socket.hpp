/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Socket.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lopoka <lopoka@student.hive.fi>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/28 12:04:58 by lopoka            #+#    #+#             */
/*   Updated: 2024/11/21 17:05:04 by lopoka           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
#ifndef SOCKET_HPP
#define SOCKET_HPP

#include <HttpServer.hpp>

class ServerConfig;

class Socket {
    private:
	int _socketFd;
	std::vector<std::shared_ptr<ServerConfig> > _servers;

    public:
	Socket(std::shared_ptr<ServerConfig> server);
	Socket(ServerConfig *server);
	~Socket();

	void close_socket(void);
	void setSocketDescriptor(int socketFd);
	void addServer(std::shared_ptr<ServerConfig> server);
	const std::vector<std::shared_ptr<ServerConfig> > getServers() const;

	int bind_socket(std::string ip, int port);
};
#endif
