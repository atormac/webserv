/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Location.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lopoka <lopoka@student.hive.fi>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/02 12:26:11 by lopoka            #+#    #+#             */
/*   Updated: 2024/11/05 19:24:16 by lopoka           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
#pragma once
#include <ServerConfig.hpp>
#include <Utils.hpp>

class	ServerConfig;

class Location
{
	private:
			ServerConfig*				_serverConfig;
			std::string 				_path;
			std::string 				_rootPath;
			bool						_autoIndex;
			std::vector<std::string>	_allowMethods;

	public :
			Location(ServerConfig *serverConfig);
			Location(const Location &original);
			~Location();
			Location &operator = (const Location &original);

			void parseLocation(std::ifstream &configFile);
			void _addAutoIndex(std::string &value);
			
			bool getAutoIndex();
};
