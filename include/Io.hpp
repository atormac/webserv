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
	int	file_type(std::string filename);
	bool	read_file(std::string filename, std::ostringstream &out);
	bool write_file(std::string filename, std::string &content);
};
#endif
