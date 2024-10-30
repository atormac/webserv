#include <Server.hpp>

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
#include <fstream>
#include <Utils.hpp>


int signo = 0;

void signal_handler(int code)
{
	if (code == SIGINT)
		signo = code;
}

Server::Server()
{
	signal(SIGINT, ::signal_handler);
}

Server::~Server()
{
	std::cout << "Server deconstructor called" << std::endl;
}

void Server::close_server(void)
{
	if (this->_epoll_fd == -1)
		return;
	for(const server_entry& entry : this->_entries)
	{
		close(entry.socket_fd);
	}
	close(this->_epoll_fd);
	this->_epoll_fd = -1;
	std::cout << "Server closed" << std::endl;
}

bool Server::set_nonblocking(int fd)
{
	int flags = fcntl(fd, F_GETFL, 0);
	if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1)
	{
		perror("fcntl");
		return false;
	}
	return true;
}

bool Server::add(std::string ip, int port)
{
	struct sockaddr_in socket_addr;
	struct server_entry entry;
	int opt = 1;

	socket_addr.sin_family = AF_INET;
	socket_addr.sin_port = htons(port);
	socket_addr.sin_addr.s_addr = inet_addr(ip.c_str());

	entry.port = port;
	entry.ip = ip;
	entry.socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (entry.socket_fd == -1)
	{
		perror("socket");
		return false;
	}
	set_nonblocking(entry.socket_fd);
	if (setsockopt(entry.socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
	{
		perror("setsockopt");
		return false;
	}
	if (bind(entry.socket_fd, (sockaddr *)&socket_addr, sizeof(socket_addr)) == -1)
	{
		perror("bind");
		return false;
	}
	if (::listen(entry.socket_fd, LISTEN_BACKLOG) == -1)
	{
		perror("listen");
		return false;
	}
	std::cout << "server: " << entry.ip << entry.port << " added" << std::endl;
	this->_entries.push_back(entry);
	return true;
}

bool Server::epoll()
{
	struct epoll_event events[MAX_EVENTS];

	this->_epoll_fd = epoll_create1(0);
	if (this->_epoll_fd == -1)
	{
		perror("epoll_create");
		return false;
	}
	for(const server_entry& entry : this->_entries)
	{
		struct epoll_event ev;
		ev.events = EPOLLIN;
		ev.data.fd = entry.socket_fd;
		if (::epoll_ctl(this->_epoll_fd, EPOLL_CTL_ADD, entry.socket_fd, &ev) == -1)
		{
			perror("epoll_ctl");
			return false;
		}
	}
	while (true)
	{
		int nfds = ::epoll_wait(this->_epoll_fd, events, MAX_EVENTS, -1);
		if (nfds == -1)
			break;
		for (int i = 0; i < nfds; i++)
		{
			bool registered = false;
			epoll_event &event = events[i];
			for(const server_entry& entry : this->_entries)
			{
				if (event.data.fd == entry.socket_fd) //Queue requests
				{
					registered = true;
					if (this->accept_client(entry.socket_fd))
					{
						std::cout << "client accepted" << std::endl;
					}
				}
			}
			if (registered)
				continue;
			if ((event.events & EPOLLIN) || (event.events & EPOLLOUT))
				this->handle_event(event);
		}
	}
	return true;
}

bool Server::accept_client(int _socket_fd)
{
	struct sockaddr_in peer_addr;
	socklen_t peer_addr_size = sizeof(peer_addr);
	struct epoll_event ev;

	int client_fd = accept(_socket_fd, (sockaddr *)&peer_addr, &peer_addr_size);
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
		remove_client((Client *)ev.data.ptr);
		return false;
	}
	return true;
}

void Server::remove_client(Client *client)
{
	if (!client)
		return;
	close(client->fd);
	delete client;
}


bool Server::handle_event(epoll_event &event)
{
	Client *client = (Client *)event.data.ptr;
	struct epoll_event ev_new;

	if (client == NULL)
		return false;
	if (event.events & EPOLLIN) //read
	{
		char buffer[2048 + 1] = { 0 };
		ssize_t bytes_read = read(client->fd, buffer, 2048);
		if (bytes_read == -1)
		{
			remove_client(client);
			return false;
		}
		std::cout << "[webserv] request received " << bytes_read << " | ";
		std::cout << client->ip_addr;
		std::cout << std::endl;
		if (client->req == nullptr)
		{
			client->req = new Request();
		}
		std::string req_str(buffer, bytes_read);
		State state = client->req->parse(req_str);
		client->req->dump();

		ev_new.events = EPOLLET | EPOLLIN;
		ev_new.data.fd = 0;
		ev_new.data.ptr = event.data.ptr;
		if (state == State::Complete || bytes_read == 0)
		{
			std::cout << "EOF\n";
			ev_new.events = EPOLLET | EPOLLOUT;
		}
	
		//std::cout << buffer << std::endl;
		if (epoll_ctl(this->_epoll_fd, EPOLL_CTL_MOD, client->fd, &ev_new) == -1)
		{
			perror("epoll_ctl");
			return false;
		}
	} else if (event.events & EPOLLOUT) //write
	{
		std::cout << "[webserv] write " << client->ip_addr << std::endl;
		Response resp(client->req);
		(void)resp;
		this->response(client->fd);
		if (epoll_ctl(this->_epoll_fd, EPOLL_CTL_DEL, client->fd, NULL) == -1)
		{
			perror("epoll_ctl");
			return false;
		}
		remove_client(client);
	}
	return true;
}

void Server::response(int client_fd)
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

void Server::parseServer(std::ifstream &configFile)
{
	std::string line;

	skipEmptyLines(configFile, line);
	if (!configFile)
		throw std::runtime_error("parseServer: Empty server block!");
	if (line != "{")
		throw std::runtime_error("parseServer: No '{' opening server block!");

	while (skipEmptyLines(configFile, line), configFile)
	{
		if (line != "}")
			std::cout << "In serv block: " << line << std::endl;
		else if (line == "}")
			break;
		else
			throw std::runtime_error("parseServer: Unknown element in server block: " + line);
	}
	if (line != "}")
		throw std::runtime_error("parseSerever: Unterminated server block!");
}
