#include <HttpServer.hpp>

void HttpServer::parseConfig(const std::string &filePath)
{
	std::ifstream configFile;
	std::string	line;

	configFile.open(filePath.c_str());
	if (!configFile.is_open())
		throw std::runtime_error("ParseConfig: Coldn't open config file");
	std::cout << "Parsing config file" << std::endl;

	while (std::getline(configFile, line))
	{
		removeComments(line);
		if (line.empty())
			continue;
		if (std::regex_match(line, std::regex("^server\\s*$")))
		{
			std::shared_ptr<ServerConfig> server(new ServerConfig());
			server->parseServerConfig(configFile);
			std::string ip_and_port = server->getIpAddress() + ":" + server->getPort();
	
			if (_portsToSockets.count(ip_and_port))
				_portsToSockets[ip_and_port]->addServer(server);
			else
			{
				std::shared_ptr<Socket> socket(new Socket(server));
				_portsToSockets.insert(std::make_pair(ip_and_port, socket));
			}

			// For testing
			//for (std::string i: server->getNames())
			//	std::cout << "Server name: " << i << std::endl;
			//std::cout << "Server max size: " << server->getMaxSize() << std::endl << std::endl;
			//
		}
		else
			throw std::runtime_error("ParseConfig: Unexpected value outside server block: " + line);
	}
	std::cout << "Config parsing completed" << std::endl;
	std::cout << "Number of sockets: " << _portsToSockets.size() << std::endl;
}