/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utils.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lopoka <lopoka@student.hive.fi>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/29 15:55:43 by lopoka            #+#    #+#             */
/*   Updated: 2024/11/05 22:47:01 by lopoka           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
#pragma once
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
size_t stoT(std::string s);
unsigned int stoUI(std::string s);

bool fileExists(const std::string &name);

#include "Utils.tpp"
