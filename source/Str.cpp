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

	std::string url_decode(std::string s)
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
}
