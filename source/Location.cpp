/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Location.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lopoka <lopoka@student.hive.fi>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/02 12:32:30 by lopoka            #+#    #+#             */
/*   Updated: 2024/11/16 16:01:53 by lopoka           ###   ########.fr       */
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
	std::regex ptrn("^\\s*autoindex\\s+(on|off)\\s*;\\s*$");
	std::smatch match_res;
	std::string string = ss.str();

	if (!std::regex_match(string, match_res, ptrn))
		throw std::runtime_error("_addAutoIndex: Expected format: \"autoindex [on/off];\"");
	if (match_res.str(1) == "on")
		_autoIndex = true;
	else if (match_res.str(1) == "off")
		_autoIndex = false;
	else
		throw std::runtime_error("_addAutoIndex: autoindex value other than 'on' or 'off'!");
	// For debugging
	std::cout << "Autoindex: " << _autoIndex << std::endl;
	//	
}

// Getters
bool Location::getAutoIndex() {return _autoIndex;}
