#ifndef STR_HPP
#define STR_HPP

#include <iostream>

namespace Str
{
	int	    decode_hex(const char *s);
	std::string url_decode(const std::string &s);
	std::string get_key_data(std::string &buf, std::string key);
	std::string safe_substr(std::string &buf, std::string before, std::string after);

	std::string trim_start(std::string &str, const std::string &needle);
}
#endif
