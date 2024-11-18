#ifndef IO_HPP
#define IO_HPP

#include <iostream>
#include <fstream>
#include <sstream>
#include <sys/stat.h>
#include <sys/types.h>

enum
{
	FS_EXISTS = 1 << 0,
	FS_ISFILE = 1 << 1,
	FS_ISDIR = 1 << 2,
	FS_READ = 1 << 3,
	FS_WRITE = 1 << 4,
};

class Io
{
	public:
		static bool	set_nonblocking(int fd);
		static int	file_stat(const std::string &filename);
		static bool	read_file(const std::string &filename, std::ostringstream &out);
		static bool	write_file(const std::string &filename, const std::string &content);
		static std::string get_file_ext(std::string uri);
};

#endif
