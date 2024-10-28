/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Socket.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lopoka <lopoka@student.hive.fi>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/28 12:04:58 by lopoka            #+#    #+#             */
/*   Updated: 2024/10/28 12:35:14 by lopoka           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
#pragma once
#include "HttpServer.hpp"
#include <vector>

class HttpServer;

class Socket
{
	private:
			int _socketFd;
			std::vector<HttpServer *> _servers;
	public:
			Socket(HttpServer *server);
			~Socket();

			void addServer(HttpServer *server);
};
