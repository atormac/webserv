#include <HttpServer.hpp>

int main(int argc, char **argv)
{
	if (argc != 2)
	{
		std::cerr << "Usage: ./webserv <config file>" << std::endl;
		return EXIT_FAILURE;
	}
	HttpServer server;

	try {
		server.parseConfig(argv[1]);
	} catch (const std::exception &e) {
		std::cerr << "Config parsing failed: " << e.what() << std::endl;
		return 1;
	}

	std::vector<std::shared_ptr<ServerConfig> > CONFIGS;
	CONFIGS = server.GETPORTS(7051);

	server.init();
	try  {
		server.epoll();
	} catch (const std::exception &e) {
		std::cerr << "Error thrown " << e.what() << std::endl;
	}
	server.close_server();

	return 0;
}
