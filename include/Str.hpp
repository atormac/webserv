#ifndef STR_HPP
#define STR_HPP

#include <iostream>

class Str
{
	public:
		static int	    decode_hex(const char *s);
		static std::string url_decode(const std::string &s);
		static std::string get_key_data(std::string &buf, std::string key);
		static std::string safe_substr(std::string &buf, std::string before, std::string after);
		static std::string trim_start(std::string &str, const std::string &needle);
};

#endif
