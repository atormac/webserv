#include <HttpServer.hpp>

Client::Client()
{
	this->instance = nullptr;
	this->req = nullptr;
	this->resp = nullptr;
	this->pid = -1;
	this->cgi_read_fd = -1;
	this->cgi_write_fd = -1;

	this->update_time();
}
Client::~Client()
{
	if (this->fd >= 0)
	{
		close(this->fd);
		this->fd = -1;
	}
	if (this->pid >= 0)
	{
		if (Cgi::finish(this->pid))
		{
			kill(this->pid, SIGTERM);
		}
		this->pid = -1;
	}
}

//normal
Client::Client(HttpServer &inst, int client_fd, int socket_fd, std::string ip)
	: Client()
{
	this->instance = &inst;
	this->conn_type = CONN_REGULAR;
	this->fd = client_fd;
	this->socket = socket_fd;
	this->ip_addr = ip;
	this->req = std::make_shared<Request>();

}

//cgi
Client::Client(HttpServer &inst, int cgi_fd)
	: Client()
{
	this->instance = &inst;
	this->conn_type = CONN_CGI;
	this->fd = cgi_fd;
	this->req = std::make_shared<Request>(true);
}

void Client::update_time(void)
{
	std::time(&this->_last_active);
}

bool Client::has_timed_out(time_t now)
{
	time_t delta = now - this->_last_active;

	if (delta > TIMEOUT_SECONDS)
	{
		std::cerr << "client timed out: " << this->fd << std::endl;
		return true;
	}
	return false;
}

void Client::cleanup_child(void)
{
	this->instance->close_connections();
	this->close_fd();
}

void Client::close_fd(void)
{
	if (this->fd >= 0)
	{
		close(this->fd);
		this->fd = -1;
	}
}
