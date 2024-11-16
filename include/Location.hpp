/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Location.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lopoka <lopoka@student.hive.fi>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/02 12:26:11 by lopoka            #+#    #+#             */
/*   Updated: 2024/11/16 16:14:11 by lopoka           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LOCATION_HPP
#define LOCATION_HPP
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
			void _addAutoIndex(std::stringstream &ss);
			
			bool getAutoIndex();
};

#endif
