#include <Str.hpp>
#include <cstring>

namespace Str
{
	int decode_hex(const char *s)
	{
		int	ret = 0;
		int	val = 0;
		char	c;

		if (*s == '\0')
			return -1;
		while (std::strchr("0123456789abcdefABCDEF", *s))
		{
			c = *s;
			if (c >= '0' && c <= '9')
				val = c - '0';
			else if (c >= 'a' && c <= 'f')
				val = c - 'a' + 10;
			else if (c >= 'A' && c <= 'F')
				val = c - 'A' + 10;
			else
				return -1;
			ret = (ret * 16) + val;
			s++;
		}
		return ret;
	}

	std::string url_decode(const std::string &s)
	{
		std::string res = "";

		for (size_t i = 0; i < s.length(); i++)
		{
			if (s[i] == '%' && (s.length() - i) >= 2) {
				int out;
				sscanf(s.substr(i+1,2).c_str(), "%x", &out);
				char ch = static_cast<char>(out);
				res += ch;
				i += 2;
				continue;
			}
			res += s[i];
		}
		return res;
	}
	std::string	get_key_data(std::string &buf, std::string key)
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

	std::string safe_substr(std::string &buf, std::string before, std::string after)
	{
		size_t pos = buf.find(before);
		if (pos == std::string::npos)
			return "";
		size_t end = buf.find(after, pos);
		if (end == std::string::npos)
			return buf.substr(pos + before.size());
		return buf.substr(pos, end - pos);
	}
}
