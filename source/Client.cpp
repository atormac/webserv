#include <HttpServer.hpp>

Client::Client()
{
	this->req = nullptr;
	this->resp = nullptr;
	this->status = CL_NORMAL;
	this->start_time = 0;
	this->pid = -1;
}
Client::~Client() {}

Client::Client(int client_fd, int socket_fd, std::string ip)
{
	this->fd = client_fd;
	this->socket = socket_fd;
	this->status = CL_NORMAL;

	std::time(&this->start_time);
	std::cout << "start_time (unix): " << this->start_time << std::endl;

	this->ip_addr = ip;
	this->req = std::make_shared<Request>();
	this->resp = nullptr;
	this->pid = -1;
	this->cgi_to[0] = -1;
	this->cgi_to[1] = -1;
	this->cgi_from[0] = -1;
	this->cgi_from[1] = -1;
}

