/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lopoka <lopoka@student.hive.fi>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/29 15:55:43 by lopoka            #+#    #+#             */
/*   Updated: 2024/11/06 22:41:38 by lopoka           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
#include <Utils.hpp>
#include <sstream>
#include <regex>

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

size_t stoT(std::string s)
{
    size_t res = 0;
    size_t maxT = std::numeric_limits<size_t>::max();
    
    for (char i : s)
    {
        if (!std::isdigit(i))
            throw std::runtime_error("stoT: Non digit character in size_t conversion!");
        if (!(res <= ((maxT - i + 48) / 10)))
            throw std::runtime_error("stoT: size_t overflow!");
        res = res * 10 + i - 48;
    }
    return res;
}

unsigned int stoUI(std::string s)
{
    unsigned int res = 0;
    unsigned int maxUI = std::numeric_limits<unsigned int>::max();
    
    for (char i : s)
    {
        if (!std::isdigit(i))
            throw std::runtime_error("stoUI: Non digit character in unsigned int conversion!");
        if (!(res <= ((maxUI - i + 48) / 10)))
            throw std::runtime_error("stoUI: unsigned int overflow!");
        res = res * 10 + i - 48;
    }
    return res;
}

bool fileExists(const std::string &name)
{
    std::ifstream file(name.c_str());
    return file.good();
}

bool validLineEnd(std::string &lineEnd, std::stringstream &ss)
{
	std::string temp;

	if (lineEnd.back() == ';' && ss.eof())
	{
		lineEnd.pop_back();
		return true;
	}
	ss >> temp;
	if (temp == ";" && ss.eof())
		return true;
	return false;
}
