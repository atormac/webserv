/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utils.tpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lopoka <lopoka@student.hive.fi>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/05 22:47:57 by lopoka            #+#    #+#             */
/*   Updated: 2024/11/05 22:48:06 by lopoka           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

template <class T> T stringToType(std::string str)
{
	std::istringstream iss(str);
	int result;
	char remain;
	if (!(iss >> result) || iss >> remain)
	{
	    throw std::runtime_error("stringToType: Failed to cast string to given type T!");
	}
	return result;

}
