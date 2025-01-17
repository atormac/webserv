#include <HttpServer.hpp>

int Str::decode_hex(const char *s, int *out_len)
{
	int ret = 0;
	int val = 0;
	char c;
	const char *charset = "0123456789abcdefABCDEF";

	if (std::strchr(charset, *s) == NULL)
		return -1;
	while (std::strchr(charset, *s))
	{
		c = *s;
		if (c >= '0' && c <= '9')
			val = c - '0';
		else if (c >= 'a' && c <= 'f')
			val = c - 'a' + 10;
		else if (c >= 'A' && c <= 'F')
			val = c - 'A' + 10;
		ret = (ret * 16) + val;
		*out_len = *out_len + 1;
		s++;
	}
	return ret;
}

std::string Str::url_decode(const std::string &s)
{
	std::string res = "";

	for (size_t i = 0; i < s.length(); i++)
	{
		if (s[i] == '%' && (s.length() - i) >= 2)
		{
			int out;
			sscanf(s.substr(i + 1, 2).c_str(), "%x", &out);
			char ch = static_cast<char>(out);
			res += ch;
			i += 2;
			continue;
		}
		res += s[i];
	}
	return res;
}

std::string Str::date_str_now(void)
{
	time_t t;
	struct tm *time_struct;
	char buf[128];

	std::time(&t);
	time_struct = std::gmtime(&t);
	std::strftime(buf, sizeof(buf) - 1, "%a, %d %b %Y %H:%M:%S GMT", time_struct);
	return std::string(buf);
}

std::string Str::date_str_hour_from_now(void)
{
	time_t t;
	struct tm *time_struct;
	char buf[128];

	std::time(&t);
	time_struct = std::gmtime(&t);
	time_struct->tm_hour = (time_struct->tm_hour + 1) % 24;
	std::strftime(buf, sizeof(buf) - 1, "%a, %d %b %Y %H:%M:%S GMT", time_struct);
	return std::string(buf);
}

std::string Str::get_key_data(std::string &buf, std::string key)
{
	size_t pos = buf.find(key + "=\"");
	if (pos == std::string::npos)
		return "";
	pos += key.size() + 2;
	size_t end = buf.find("\"", pos);
	if (end == std::string::npos)
		return "";
	return buf.substr(pos, end - pos);
}

std::string Str::safe_substr(std::string &buf, std::string before, std::string after)
{
	size_t pos = buf.find(before);
	if (pos == std::string::npos)
		return "";
	size_t end = buf.find(after, pos);
	if (end == std::string::npos)
		return buf.substr(pos + before.size());
	return buf.substr(pos, end - pos);
}

std::string Str::trim_start(std::string &str, const std::string &needle)
{
	std::string result = str;
	size_t pos = result.find(needle);

	if (pos == std::string::npos)
		return result;
	result.erase(0, pos + needle.size());
	return result;
}


int Str::content_len_int(const std::string& input) {
	int result = -1;
	try {
		result = std::stoul(input);
	} catch (const std::invalid_argument& e) {
		return -1;
	} catch (const std::out_of_range& e) {
		return -1;
	}
	return result;
}
