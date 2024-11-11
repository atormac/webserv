#include <Client.hpp>

Client::Client()
{
	this->req = nullptr;
}
Client::~Client()
{
}

Client::Client(int client_fd, std::string ip)
{
	this->fd = client_fd;
	this->ip_addr = ip;
	this->req = std::make_shared<Request>();
}

