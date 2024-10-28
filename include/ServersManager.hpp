/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServersManager.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lopoka <lopoka@student.hive.fi>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/28 11:17:19 by lopoka            #+#    #+#             */
/*   Updated: 2024/10/28 15:10:32 by lopoka           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
#pragma once
#include <map>
#include "Server.hpp"
#include "Socket.hpp"

class Socket;

class ServersManager
{
	private:
			std::map<std::string, Socket *> _portsToSockets;
			
			bool _socketPresent(std::string &port);

	public:
			~ServersManager();			

			void addSocket(std::string &port, Server *server);
};
