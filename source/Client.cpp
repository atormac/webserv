#include <Client.hpp>

Client::Client()
{
	this->req = nullptr;
	this->is_cgi = false;
	this->start_time = 0;
}
Client::~Client() {}

Client::Client(int client_fd, int socket_fd, std::string ip)
{
	this->fd = client_fd;
	this->socket = socket_fd;
	this->is_cgi = false;

	std::time(&this->start_time);
	std::cout << "start_time (unix): " << this->start_time << std::endl;

	this->ip_addr = ip;
	this->req = std::make_shared<Request>();
}

