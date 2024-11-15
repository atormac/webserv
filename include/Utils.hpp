/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utils.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lopoka <lopoka@student.hive.fi>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/29 15:55:43 by lopoka            #+#    #+#             */
/*   Updated: 2024/11/15 17:59:41 by atorma           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
#ifndef UTILS_HPP
#define UTILS_HPP
#include <string>
#include <iostream>
#include <fstream>
#include <limits>

void removeComments(std::string &line);
void skipEmptyLines(std::ifstream &configFile, std::string &line);

std::string leftWspcTrim(std::string string);
std::string rightWspcTrim(std::string string);
std::string WspcTrim(std::string string);

size_t stoT(std::string s);
unsigned int stoUI(std::string s);

bool fileExists(const std::string &name);
#endif
