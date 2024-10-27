#include "../include/Client.hpp"

Client::Client(int client_fd, std::string ip)
{
	this->fd = client_fd;
	this->ip_addr = ip;
}

Client::~Client()
{
}
