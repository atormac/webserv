/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utils.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lopoka <lopoka@student.hive.fi>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/29 15:55:43 by lopoka            #+#    #+#             */
/*   Updated: 2024/10/31 19:49:30 by lopoka           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
#pragma once
#include <string>
#include <iostream>
#include <fstream>
#include <limits>

void removeComments(std::string &line);
void skipEmptyLines(std::ifstream &configFile, std::string &line);

std::string leftWspcTrim(std::string string);
std::string rightWspcTrim(std::string string);
std::string WspcTrim(std::string string);

size_t stot(std::string s);
