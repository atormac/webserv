#ifndef IO_HPP
#define IO_HPP

#include <iostream>
#include <fstream>
#include <sstream>
#include <sys/stat.h>
#include <sys/types.h>

enum
{
	FILE_NOT_EXISTS,
	FILE_FILE,
	FILE_DIRECTORY,
};

namespace Io
{
	bool	set_nonblocking(int fd);
	int	file_type(const std::string &filename);
	bool	read_file(const std::string &filename, std::ostringstream &out);
	bool	write_file(const std::string &filename, std::string &content);
	std::string get_file_ext(std::string uri);
};
#endif
