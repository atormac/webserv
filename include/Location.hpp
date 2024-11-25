/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Location.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lopoka <lopoka@student.hive.fi>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/02 12:26:11 by lopoka            #+#    #+#             */
/*   Updated: 2024/11/25 18:46:00 by atorma           ###   ########.fr       */
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
	public :
			ServerConfig*						_serverConfig;
			std::string 						_path;
			std::string 						_rootPath;
			std::vector<std::string>			_indices;
			bool								_autoIndex;
			bool								_autoIndexSet;
			std::vector<std::string>			_methods;
			int									_redirectCode;
			std::string							_redirectPath;
			std::string							_uploadPath;
			std::map<std::string, std::string>	_cgi;

			Location(ServerConfig *serverConfig);
			Location(const Location &original);
			Location();
			~Location();
			Location &operator = (const Location &original);

			void parseLocation(std::ifstream &configFile, std::string &location_line);
			void _addPath(std::string &line);
			void _addAutoIndex(std::string &line);
			void _addRoot(std::string &line);
			void _addIndex(std::string &line);
			void _addMethods(std::string &line);
			void _addRedirect(std::string &line);
			void _addUpload(std::string &line);
			void _addCgi(std::string &line);
			
			bool getAutoIndex();
			void dump(void);
};

#endif
