/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lopoka <lopoka@student.hive.fi>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/29 15:55:43 by lopoka            #+#    #+#             */
/*   Updated: 2024/10/30 16:00:43 by lopoka           ###   ########.fr       */
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
