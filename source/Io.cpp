#include <HttpServer.hpp>

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

int Io::file_stat(const std::string &filename)
{
	int flags = 0;
	struct stat sb;

	if (stat(filename.c_str(), &sb) != 0)
		return flags;
	if (S_ISREG(sb.st_mode))
		flags |= FS_ISFILE;
	if (S_ISDIR(sb.st_mode))
		flags |= FS_ISDIR;
	if (access(filename.c_str(), R_OK) == 0)
		flags |= FS_READ;
	if (access(filename.c_str(), W_OK) == 0)
		flags |= FS_WRITE;
	return flags;
}

bool Io::read_file(const std::string &filename, std::ostringstream &out)
{
	std::ifstream file(filename, std::ios::binary);
	if (!file || file.fail())
		return false;
	out << file.rdbuf();
	file.close();
	return true;
}

bool Io::write_file(const std::string &filename, const std::string &content)
{
	std::ofstream file(filename, std::ios::out | std::ios::binary | std::ios::trunc);
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
