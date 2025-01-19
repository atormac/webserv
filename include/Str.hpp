#ifndef STR_HPP
#define STR_HPP

#include <HttpServer.hpp>

class Str {
    public:
	static int decode_hex(const char *s, int *out_len);
	static std::string url_decode(const std::string &s);
	static std::string date_str_now(void);
	static std::string date_str_hour_from_now(void);
	static std::string time_to_str(time_t t);
	static std::string get_key_data(std::string &buf, std::string key);
	static std::string safe_substr(std::string &buf, std::string before,
				       std::string after);
	static std::string trim_start(std::string &str, const std::string &needle);
	static int content_len_int(const std::string& input);
};

#endif
