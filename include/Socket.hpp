/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Socket.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lopoka <lopoka@student.hive.fi>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/28 12:04:58 by lopoka            #+#    #+#             */
/*   Updated: 2025/01/10 23:04:02 by user             ###   ########.fr       */
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
	bool serverExist(std::vector<std::string> names) const;
	const std::vector<std::shared_ptr<ServerConfig> > getServers() const;

	int bind_socket(std::string ip, int port);
};
#endif
