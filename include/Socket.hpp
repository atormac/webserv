/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Socket.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lopoka <lopoka@student.hive.fi>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/28 12:04:58 by lopoka            #+#    #+#             */
/*   Updated: 2024/11/17 16:03:31 by user             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
#ifndef SOCKET_HPP
#define SOCKET_HPP
#include <ServerConfig.hpp>
#include <vector>

class ServerConfig;

class Socket
{
	private:
			int _socketFd;
			std::vector<ServerConfig *> _servers;
	public:
			Socket(ServerConfig *server);
			~Socket();
			
			void	close_socket(void);
			void setSocketDescriptor(int socketFd);
			void addServer(ServerConfig *ServerConfig);
			const std::vector<ServerConfig *> getServers() const;

			int  bind_socket(std::string ip, int port);
};
#endif
