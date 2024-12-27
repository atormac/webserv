#include <HttpServer.hpp>

Client::Client()
{
	this->req = nullptr;
	this->resp = nullptr;
	this->ref = nullptr;
	this->pid = -1;
	this->cgi_to[0] = -1;
	this->cgi_to[1] = -1;
	this->cgi_from[0] = -1;
	this->cgi_from[1] = -1;

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

	this->update_time();
}

//cgi
Client::Client(int client_fd, int pid, std::shared_ptr<Client> ref_ptr) : Client()
{
	this->conn_type = CONN_CGI;
	this->fd = client_fd;
	this->pid = pid;
	this->ref = ref_ptr;
	this->req = std::make_shared<Request>();

	this->update_time();
}

void Client::update_time(void)
{
	std::time(&this->_last_active);
}

bool Client::has_timed_out(void)
{
	time_t now;
	std::time(&now);

	time_t delta = now - this->_last_active;

	if (delta > TIMEOUT_SECONDS) {
		std::cout << "client timed out: " << this->fd << std::endl;
		return true;
	}
	return false;
}

