#ifndef IO_HPP
#define IO_HPP

#include <iostream>
#include <fstream>
#include <sstream>

namespace Io
{
	bool	read_file(std::string filename, std::ostringstream &out);
	bool write_file(std::string filename, std::string &content);
};
#endif
