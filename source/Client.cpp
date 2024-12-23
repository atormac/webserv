#include <HttpServer.hpp>

Client::Client()
{
	this->req = nullptr;
	this->resp = nullptr;
	this->ref = nullptr;
	this->start_time = 0;
	this->pid = -1;
	this->cgi_to[0] = -1;
	this->cgi_to[1] = -1;
	this->cgi_from[0] = -1;
	this->cgi_from[1] = -1;

	std::time(&this->start_time);
}
Client::~Client() {}

//normal
Client::Client(int client_fd, int socket_fd, std::string ip) : Client()
{
	this->conn_type = CONN_REGULAR;
	this->fd = client_fd;
	this->socket = socket_fd;
	this->ip_addr = ip;
	this->req = std::make_shared<Request>();
}

//cgi
Client::Client(int client_fd, int pid, Client *ref) : Client()
{
	this->conn_type = CONN_CGI;
	this->fd = client_fd;
	this->pid = pid;
	this->ref = ref;
	this->req = std::make_shared<Request>();
}

