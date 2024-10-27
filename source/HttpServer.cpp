#include "../include/HttpServer.hpp"
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <iostream>
#include <stdlib.h>
#include <sstream>
#include <sys/epoll.h>
#include <signal.h>

int signo = 0;
void signal_handler(int code)
{
	signo = code;
}

HttpServer::HttpServer(std::string ip, int port)
{
	this->_port = port;
	this->_ip = ip;
	this->_socket_fd = -1;
	this->_epoll_fd = -1;
}

HttpServer::~HttpServer()
{
	if (this->_epoll_fd >= 0)
		close(this->_epoll_fd);
	if (this->_socket_fd >= 0)
		close(this->_socket_fd);
	this->_socket_fd = -1;
	this->_epoll_fd = -1;
	std::cout << "HttpServer deconstructor called" << std::endl;
}

bool HttpServer::set_nonblocking(int fd)
{
	int flags = fcntl(fd, F_GETFL, 0);
	if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1)
	{
		perror("fcntl");
		return false;
	}
	return true;
}
bool HttpServer::init(void)
{
	struct sockaddr_in socket_addr;
	int opt = 1;

	signal(SIGINT, ::signal_handler);
	//memset(&socket_addr, 0, sizeof(socket_addr));
	socket_addr.sin_family = AF_INET;
	socket_addr.sin_port = htons(this->_port);
	socket_addr.sin_addr.s_addr = inet_addr(this->_ip.c_str());

	int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_fd == -1)
	{
		perror("socket");
		return false;
	}
	if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
	{
		perror("setsockopt");
		return false;
	}
	if (bind(socket_fd, (sockaddr *)&socket_addr, sizeof(socket_addr)) == -1)
	{
		perror("bind");
		return false;
	}
	this->_socket_fd = socket_fd;
	return true;
}

bool HttpServer::listen()
{
	struct epoll_event ev;
	struct epoll_event events[MAX_EVENTS];

	if (::listen(this->_socket_fd, LISTEN_BACKLOG) == -1)
	{
		perror("listen");
		return false;
	}
	std::cout << "[webserv] server listening on port: " << this->_port << std::endl;

	this->_epoll_fd = epoll_create1(0);
	if (this->_epoll_fd == -1)
	{
		perror("epoll_create");
		return false;
	}
	ev.events = EPOLLIN;
	ev.data.fd = this->_socket_fd;
	if (::epoll_ctl(this->_epoll_fd, EPOLL_CTL_ADD, this->_socket_fd, &ev) == -1)
	{
		perror("epoll_ctl");
		return false;
	}
	while (true)
	{
		int nfds = ::epoll_wait(this->_epoll_fd, events, MAX_EVENTS, -1);

		if (nfds == -1)
			break;
		for (int i = 0; i < nfds; i++)
		{
			epoll_event &event = events[i];
			if (event.data.fd == this->_socket_fd) //Queue requests
				this->accept_client();
			else if ((event.events & EPOLLIN) || (event.events & EPOLLOUT))
				this->handle_event(event);
		}
	}
	return true;
}

bool HttpServer::accept_client(void)
{
	struct sockaddr_in peer_addr;
	socklen_t peer_addr_size = sizeof(peer_addr);
	struct epoll_event ev;

	int client_fd = accept(this->_socket_fd, (sockaddr *)&peer_addr, &peer_addr_size);
	if (client_fd == -1)
	{
		std::cerr << "accept() failed" << std::endl;
		return false;
	}
	set_nonblocking(client_fd);
	ev.events = EPOLLIN | EPOLLET;
	ev.data.fd = 0;
	ev.data.ptr = new Client(client_fd, inet_ntoa(peer_addr.sin_addr));
	if (epoll_ctl(this->_epoll_fd, EPOLL_CTL_ADD, client_fd, &ev) == -1)
	{
		std::cerr << "epoll_ctl accept failed" << std::endl;
		return false;
	}
	return true;
}

bool HttpServer::handle_event(epoll_event &event)
{
	Client *client = (Client *)event.data.ptr;
	struct epoll_event ev_new;

	if (client == NULL)
		return false;
	if (event.events & EPOLLIN) //read
	{
		char buffer[2048 + 1] = { 0 };
		ssize_t bytes_read = read(client->fd, buffer, 2048);
		if (bytes_read < 0)
		{
			perror("read");
			return false;
		}
		std::cout << "[webserv] request received " << client->ip_addr
			  << std::endl;
		std::cout << buffer << std::endl;
		ev_new.events = EPOLLOUT | EPOLLET;
		ev_new.data.fd = 0;
		ev_new.data.ptr = event.data.ptr;
		if (epoll_ctl(this->_epoll_fd, EPOLL_CTL_MOD, client->fd, &ev_new) == -1)
		{
			perror("epoll_ctl");
			return false;
		}
	} else if (event.events & EPOLLOUT) //write
	{
		std::cout << "[webserv] write " << client->ip_addr << std::endl;
		this->response(client->fd);
		if (epoll_ctl(this->_epoll_fd, EPOLL_CTL_DEL, client->fd, NULL) == -1)
		{
			perror("epoll_ctl");
			return false;
		}

		close(client->fd);
		delete client;
	}
	return true;
}

void HttpServer::response(int client_fd)
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
