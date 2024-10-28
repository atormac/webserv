/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Socket.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lopoka <lopoka@student.hive.fi>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/28 12:04:58 by lopoka            #+#    #+#             */
/*   Updated: 2024/10/28 17:42:59 by lopoka           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
#pragma once
#include <Server.hpp>
#include <vector>

class Server;

class Socket
{
	private:
			int _socketFd;
			std::vector<Server *> _servers;
	public:
			Socket(Server *server);
			~Socket();
			
			void setSocketDescriptor(int socketFd);
			void addServer(Server *server);
			const std::vector<Server *> getServers() const;
};
