#include <Io.hpp>
#include <fcntl.h>
#include <unistd.h>

bool Io::set_nonblocking(int fd)
{
	int flags = 0;

	if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1)
	{
		perror("fcntl");
		return false;
	}
	return true;
}

int	Io::file_stat(const std::string &filename)
{
	struct stat sb;

	if(stat(filename.c_str(), &sb) != 0)
		return FILE_NOT_EXISTS;
	if (access(filename.c_str(), R_OK) != 0)
		return FILE_NOT_EXISTS;
	if (S_ISREG(sb.st_mode))
		return FILE_FILE;
	if (S_ISDIR(sb.st_mode))
		return FILE_DIRECTORY;
	return FILE_NOT_EXISTS;
}

bool	Io::read_file(const std::string &filename, std::ostringstream &out)
{
	std::ifstream file(filename, std::ios::binary);
	if (!file || file.fail())
		return false;
	out << file.rdbuf();
	file.close();
	return true;
}

bool	Io::write_file(const std::string &filename, const std::string &content)
{
	//could use try catch here
	std::ofstream file(filename, std::ios::out | std::ios::binary | std::ios::app);
	if (!file || file.fail())
		return false;
	file.write(content.data(), content.size());
	if (file.fail())
	{
		file.close();
		return false;
	}
	file.close();
	return true;
}

std::string Io::get_file_ext(std::string uri)
{
	size_t pos = uri.find_last_of(".");

	if (pos != std::string::npos)
	{
		return uri.substr(pos);
	}
	return "";
}
