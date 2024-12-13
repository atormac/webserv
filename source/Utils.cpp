/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lopoka <lopoka@student.hive.fi>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/29 15:55:43 by lopoka            #+#    #+#             */
/*   Updated: 2024/11/16 21:44:57 by lopoka           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
#include <HttpServer.hpp>

void removeComments(std::string &line)
{
	line = std::regex_replace(line, std::regex("#.*$"), "");
}

void skipEmptyLines(std::ifstream &configFile, std::string &line)
{
	while ((std::getline(configFile, line)) && (removeComments(line), WspcTrim(line).empty()));
}

std::string leftWspcTrim(std::string string)
{
	const char *ws = " \t\n\r\f\v";
	string.erase(0, string.find_first_not_of(ws));
	return string;
}

std::string rightWspcTrim(std::string string)
{
	const char *ws = " \t\n\r\f\v";
	string.erase(string.find_last_not_of(ws) + 1);
	return string;
}

std::string WspcTrim(std::string string)
{
	return leftWspcTrim(rightWspcTrim(string));
}

bool fileExists(const std::string &name)
{
    std::ifstream file(name.c_str());
    return file.good();
}
