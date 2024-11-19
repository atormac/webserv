/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Location.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lopoka <lopoka@student.hive.fi>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/02 12:32:30 by lopoka            #+#    #+#             */
/*   Updated: 2024/11/19 10:41:14 by lopoka           ###   ########.fr       */
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

	skipEmptyLines(configFile, line);
	if (!configFile)
		throw std::runtime_error("parseLocation: Empty location block!");
	if (!std::regex_match(line, std::regex("\t\\{\\s*")))
		throw std::runtime_error("parseLocation: No '{' opening location block!");

	while (skipEmptyLines(configFile, line), configFile)
	{
		std::smatch match_res;
		std::regex ptrn("^\t{2}(\\w+).*");
		if (!std::regex_match(line, match_res, ptrn))
			break;

		if (match_res[1] != "}" && match_res[1] != "autoindex" && match_res[1] != "root")
			std::cout << "In location block: |" << match_res[1] << "|" << std::endl;
		else if (match_res[1] == "root")
			_addRoot(line);
		else if (match_res[1] == "autoindex")
			_addAutoIndex(line);
		else
			throw std::runtime_error("parseLocation: Unknown element in location block: " + line);	
	}
	if (!std::regex_match(line, std::regex("\t\\}\\s*")))
		throw std::runtime_error("parseLocation: No '}' closing location block!");
}

// Setters
void Location::_addRoot(std::string line)
{	
	std::regex ptrn("\t{2}root\\s+(.*)\\s*;\\s*");
	std::smatch match_res;
	struct stat mode;

	if (!std::regex_match(line, match_res, ptrn))
		throw std::runtime_error("_addRoot: Expected format: \"root [directory];\"");
	if (stat(match_res.str(1).c_str(), &mode) != 0)
        throw(std::runtime_error("_addRoot: Specified path doesn't exist!"));
	if (!(mode.st_mode & S_IFDIR))
		throw(std::runtime_error("_addRoot: Specified path isn't a directory!"));
	_rootPath = match_res[1];
	// For debugging
	std::cout << "root path: " << _rootPath << std::endl;
	//	
}

void Location::_addAutoIndex(std::string &line)
{
	std::regex ptrn("^\\s*autoindex\\s+(on|off)\\s*;\\s*$");
	std::smatch match_res;

	if (!std::regex_match(line, match_res, ptrn))
		throw std::runtime_error("_addAutoIndex: Expected format: \"autoindex [on/off];\"");
	if (match_res[1] == "on")
		_autoIndex = true;
	else if (match_res[1] == "off")
		_autoIndex = false;
	else
		throw std::runtime_error("_addAutoIndex: autoindex value other than 'on' or 'off'!");
	// For debugging
	std::cout << "Autoindex: " << _autoIndex << std::endl;
	//	
}

// Getters
bool Location::getAutoIndex() {return _autoIndex;}
