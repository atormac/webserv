#include <Io.hpp>

namespace Io
{

	bool	read_file(std::string filename, std::ostringstream &out)
	{
		std::ifstream file(filename, std::ios::binary);
		if (!file)
			return false;
		out << file.rdbuf();
		file.close();
		return true;
	}

	bool	write_file(std::string filename, std::string &content)
	{
		//could use try catch here
		std::ofstream fs(filename, std::ios::out | std::ios::binary | std::ios::app);
		if (fs.fail())
			return false;
		fs.write(content.data(), content.size());
		if (fs.fail())
		{
			fs.close();
			return false;
		}
		fs.close();
		return true;
	}
};
