/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Location.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lopoka <lopoka@student.hive.fi>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/02 12:32:30 by lopoka            #+#    #+#             */
/*   Updated: 2024/11/06 20:21:12 by lopoka           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
#include "Location.hpp"
#include <regex>

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
	std::string element;

	skipEmptyLines(configFile, line);
	if (!configFile)
		throw std::runtime_error("parseLocation: Empty location block!");
	if (WspcTrim(line) != "{")
		throw std::runtime_error("parseLocation: No '{' opening location block!");

	while (skipEmptyLines(configFile, line), configFile)
	{
		std::stringstream ss(line);
		ss >> element;

		if (element != "}" && element != "autoindex")
			std::cout << "In location block: |" << element << "|" << std::endl;
		else if (element == "autoindex")
			_addAutoIndex(ss);
		else if (element == "}")
			break;
		else
			throw std::runtime_error("parseLocation: Unknown element in location block: " + line);	
	}
	//if (line != "}")
	if (!std::regex_match(line, std::regex("\\s*}")))
	{
		std::cout << "Location endline |" << line << "|" << std::endl;
		throw std::runtime_error("parseLocation: Unterminated location block!");
	}
}

// Setters
void Location::_addAutoIndex(std::stringstream &ss)
{
	
	std::string value;

	ss >> value;
	if (value.empty())
		throw std::runtime_error("_addAutoIndex: Adding empty max size!");
	if (!validLineEnd(value, ss))
		throw std::runtime_error("_addAutoIndex: Unexpected characters in max size line!");
	
	if (value == "on")
		_autoIndex = true;
	else if (value == "off")
		_autoIndex = false;
	else
		throw std::runtime_error("_addAutoIndex: autoindex value other than 'on' or 'off'!");
}

// Getters
bool Location::getAutoIndex() {return _autoIndex;}
