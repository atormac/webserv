/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Location.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lopoka <lopoka@student.hive.fi>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/02 12:32:30 by lopoka            #+#    #+#             */
/*   Updated: 2024/11/04 20:59:08 by lopoka           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
#include "Location.hpp"

Location::Location(ServerConfig *srvConf): _serverConfig(srvConf), _autoIndex(false) {}

Location::Location(const Location &original): _serverConfig(original._serverConfig),
											_path(original._path),
											_rootPath(original._rootPath),
											_autoIndex(original._autoIndex),
											_allowMethods(original._allowMethods) {}

Location::~Location(){}

Location &Location::operator = (const Location &original)
{
	if (this != &original)
	{
		this->~Location();
		new(this) Location(original);
	}
	return (*this);
}

void Location::parseLocation(std::ifstream &configFile)
{
	std::string line;

	skipEmptyLines(configFile, line);
	if (!configFile)
		throw std::runtime_error("parseLocation: Empty location block!");
	if (WspcTrim(line) != "{")
		throw std::runtime_error("parseLocation: No '{' opening location block!");

	while (skipEmptyLines(configFile, line), configFile)
	{
		line = WspcTrim(line);
		size_t delim = line.find(" ");
		std::string element = (delim != std::string::npos) ? line.substr(0, delim) : line;
		std::string value = (delim != std::string::npos) ? WspcTrim(line.substr(delim + 1)) : "";
	
		if (line != "}" && line != "autoindex")
			std::cout << "In location block: |" << element << "|" << std::endl;
		else if (line == "autoindex")
			std::cout << "here autoindex" << std::endl;
		else if (line == "}")
			break;
		else
			throw std::runtime_error("parseLocation: Unknown element in location block: " + line);	
	}
	if (line != "}")
		throw std::runtime_error("parseLocation: Unterminated location block!");
}
