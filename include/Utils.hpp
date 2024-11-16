/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utils.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lopoka <lopoka@student.hive.fi>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/29 15:55:43 by lopoka            #+#    #+#             */
/*   Updated: 2024/11/16 21:45:44 by lopoka           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
#ifndef UTILS_HPP
#define UTILS_HPP
#include <string>
#include <iostream>
#include <fstream>
#include <limits>
#include <sstream>

void removeComments(std::string &line);
void skipEmptyLines(std::ifstream &configFile, std::string &line);

std::string leftWspcTrim(std::string string);
std::string rightWspcTrim(std::string string);
std::string WspcTrim(std::string string);

template <class T> T stringToType(std::string str);

bool fileExists(const std::string &name);

#include "Utils.tpp"
#endif
