#include <HttpServer.hpp>

void HttpServer::parseConfig(const std::string &filePath)
{
	std::ifstream configFile;
	std::string line;

	configFile.open(filePath.c_str());
	if (!configFile.is_open())
		throw std::runtime_error("ParseConfig: Coldn't open config file");
	std::cout << "Parsing config file" << std::endl;
	if (configFile.peek() == EOF)
		throw std::runtime_error("ParseConfig: Empty config file");

	while (std::getline(configFile, line))
	{
		removeComments(line);
		if (line.empty()) 
			continue;

		if (std::regex_match(line, std::regex("^server\\s*$")))
		{
			std::shared_ptr<ServerConfig> server(new ServerConfig());
			server->parseServerConfig(configFile);
			std::string ip_and_port =
				server->getIpAddress() + ":" + server->getPort();

			if (_portsToSockets.count(ip_and_port))
			{
				if (!_portsToSockets[ip_and_port]->serverExist(server->getNames()))
					_portsToSockets[ip_and_port]->addServer(server);
				else
					throw std::runtime_error("ParseConfig: Adding duplicate servers(same IP, Port and Name)");
			}
			else
			{
				std::shared_ptr<Socket> socket(new Socket(server));
				_portsToSockets.insert(
					std::make_pair(ip_and_port, socket));
			}
		} else
			throw std::runtime_error(
				"ParseConfig: Unexpected value outside server block: " +
				line);
	}

	if (_portsToSockets.empty())
		throw std::runtime_error("ParseConfig: No server blocks in config");
}
