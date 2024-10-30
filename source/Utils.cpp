/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lopoka <lopoka@student.hive.fi>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/29 15:55:43 by lopoka            #+#    #+#             */
/*   Updated: 2024/10/30 20:16:56 by lopoka           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
#include <Utils.hpp>

void removeComments(std::string &line)
{
	size_t trim = std::min(line.find(";"), line.find("#"));
	if (trim != std::string::npos)
		line = line.substr(0, (line[trim] == ';' ? trim + 1 : trim));
}

void skipEmptyLines(std::ifstream &configFile, std::string &line)
{
	while ((std::getline(configFile, line)) && (removeComments(line), line.empty()));
}

std::string &leftWspcTrim(std::string &string)
{
	const char *ws = " \t\n\r\f\v";
	string.erase(0, string.find_first_not_of(ws));
	return string;
}

std::string &rightWspcTrim(std::string &string)
{
	const char *ws = " \t\n\r\f\v";
	string.erase(string.find_last_not_of(ws) + 1);
	return string;
}

std::string& WspcTrim(std::string &string)
{
	return leftWspcTrim(rightWspcTrim(string));
}

