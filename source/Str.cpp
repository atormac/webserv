#include <Str.hpp>
namespace Str
{
	int decode_hex(const char *s)
	{
		int	ret = 0;
		int	val = 0;
		char	c;

		if (*s == '\0')
			return -1;
		while (*s)
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
}
