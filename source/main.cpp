#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <iostream>
#include <stdbool.h>
#include <stdlib.h>
#include <sstream>
#include <sys/epoll.h>
#include <signal.h>

#define MAX_EVENTS 10
#define LISTEN_BACKLOG 32

struct epoll_event ev[MAX_EVENTS];
struct epoll_event events[MAX_EVENTS];

void httpserver_response(int client_fd);

int socket_fd = -1;
void	signal_handler(int code)
{
	(void)code;
	if (socket_fd >= 0)
		close(socket_fd);
	socket_fd = -1;
}
bool httpserver_start(std::string ip, int port)
{
	struct sockaddr_in socket_addr;
	struct sockaddr_in peer_addr;

	//memset(&socket_addr, 0, sizeof(socket_addr));
	socket_addr.sin_family = AF_INET;
	socket_addr.sin_port = htons(port);
	socket_addr.sin_addr.s_addr = inet_addr(ip.c_str());

	socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_fd == -1)
	{
		std::cerr << "socket() failed" << std::endl;
		return false;
	}
	if (bind(socket_fd, (sockaddr *)&socket_addr, sizeof(socket_addr)) == -1)
	{
		std::cerr << "bind() failed" << std::endl;
		return false;
	}
	if (listen(socket_fd, LISTEN_BACKLOG) == -1)
	{
		std::cerr << "listen() failed" << std::endl;
		return false;
	}
	std::cout << "[webserv] server listening on port: " << port << std::endl;

	while (true)
	{
		char buffer[2048 + 1] = { 0 };

		socklen_t peer_addr_size = sizeof(peer_addr);
		int client_fd =
			accept(socket_fd, (sockaddr *)&peer_addr, &peer_addr_size);
		if (client_fd == -1)
		{
			std::cerr << "accept() failed" << std::endl;
			return false;
		}
		ssize_t bytes_read = read(client_fd, buffer, 2048);
		if (bytes_read < 0)
		{
			std::cerr << "read() failed" << std::endl;
			return false;
		}
		std::cout << "[webserv] request received" << std::endl;
		std::cout << buffer << std::endl;
		httpserver_response(client_fd);

		close(client_fd);
	}
	if (socket_fd >= 0)
		close(socket_fd);
	return true;
}

void httpserver_response(int client_fd)
{
	std::string html =
		"<!DOCTYPE html><html lang=\"en\"><body><h1>[WEBSERVER]</h1><p>HELLO WORLD</p></body></html>";
	std::ostringstream ss;
	ss << "HTTP/1.1 200 OK\nContent-Type: text/html\nContent-Length: " << html.size()
	   << "\n\n"
	   << html;
	std::string html_response = ss.str();

	write(client_fd, html_response.c_str(), html_response.size());
}

int main(void)
{
	signal(SIGINT, signal_handler);
	std::cout << "[webserv]" << std::endl;
	httpserver_start("127.0.0.1", 7051);
	return 0;
}
