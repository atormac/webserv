#include <HttpServer.hpp>

HttpServer::HttpServer()
{
	this->_epoll_fd = -1;
	this->_clients.reserve(512);
	this->_cgi_to_client.reserve(512);
}

HttpServer::~HttpServer()
{
}

int signo = 0;

void HttpServer::signal_handler(int code)
{
	if (code == SIGINT)
		signo = code;
}

bool HttpServer::init()
{
	signal(SIGPIPE, SIG_IGN);
	if (signal(SIGINT, HttpServer::signal_handler) == SIG_ERR)
		return false;

	for (std::map<std::string, std::shared_ptr<Socket> >::iterator itr =
		     _portsToSockets.begin();
	     itr != _portsToSockets.end(); itr++)
	{
		std::regex ptrn("(.*):(.*)");
		std::smatch match_res;
		std::regex_match(itr->first, match_res, ptrn);
		std::string ip = match_res[1];
		int port = stringToType<int>(match_res[2]);

		int socketFd = itr->second->bind_socket(ip, port);
		if (socketFd < 0)
			return false;
		_socketToPort.emplace(socketFd, port);
		itr->second->setSocketDescriptor(socketFd);
		_socketFdToSockets.insert({ socketFd, itr->second });
	}

	_epoll_fd = epoll_create(1);

	if (_epoll_fd == -1)
	{
		perror("epoll_create");
		return false;
	}
	for (const auto &so : _socketFdToSockets)
	{
		struct epoll_event ev;
		ev.events = EPOLLIN;
		ev.data.fd = so.first;

		if (::epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, so.first, &ev) == -1)
		{
			perror("epoll_ctl");
			return false;
		}
	}
	return true;
}

void HttpServer::cull_clients(void)
{
	time_t now;
	std::time(&now);

	std::vector <int> timedout;

	for (auto const &cl : _clients)
	{
		if (cl.second->has_timed_out(now))
		{
			timedout.push_back(cl.first);
		}
	}
	for (int i : timedout)
	{
		remove_fd(i);
	}
}

void HttpServer::epoll(void)
{
	struct epoll_event events[MAX_EVENTS];

	while (!signo)
	{
		int timeout = 1 * 1000;
		int nfds = ::epoll_wait(_epoll_fd, events, MAX_EVENTS, timeout);
	
		cull_clients();

		for (int i = 0; i < nfds; i++)
		{
			epoll_event &e = events[i];

			if (this->_socketFdToSockets.count(e.data.fd))
			{
				accept_client(e.data.fd);
				continue;
			}
			if ((e.events & EPOLLERR) || (e.events & EPOLLRDHUP))
			{
				perror("epoll_wait");
				remove_fd(e.data.fd);
				continue;
			}
			if (!_clients.count(e.data.fd))
				continue;
			std::shared_ptr cl = _clients[e.data.fd];

			cl->update_time();
			//on pipe close EPOLLHUP means EOF here
			//mark the event as OK to catch errors directly from read/write
			//calls
			if (e.events & EPOLLHUP)
				e.events |= EPOLLIN;

			if (e.events & EPOLLIN)
				handle_read(cl);
			else if (e.events & EPOLLOUT)
				handle_write(cl);
		}
	}
}

bool HttpServer::accept_client(int _socket_fd)
{
	struct sockaddr_in peer_addr;

	std::memset(&peer_addr, 0, sizeof(sockaddr_in));
	socklen_t peer_addr_size = sizeof(peer_addr);

	int client_fd = accept(_socket_fd, (sockaddr *)&peer_addr, &peer_addr_size);
	if (client_fd == -1)
	{
		perror("accept()");
		return false;
	}
	if (_clients.size() >= 512)
	{
		std::cerr << "Error: too many connections\n";
		close(client_fd);
		return false;
	}
	if (!Io::set_nonblocking(client_fd))
	{
		perror("set_nonblocking");
		close(client_fd);
		return false;
	}
	std::shared_ptr cl = std::make_shared<Client>(*this, client_fd, _socket_fd,
						      inet_ntoa(peer_addr.sin_addr));
	return mod_fd(client_fd, EPOLL_CTL_ADD, EPOLLIN, cl);
}

bool HttpServer::mod_fd(int fd, int ctl, int mask, std::shared_ptr<Client> cl)
{
	struct epoll_event ev;

	ev.events = mask;
	ev.data.fd = fd;

	if (epoll_ctl(this->_epoll_fd, ctl, fd, &ev) == -1)
	{
		perror("epoll_ctl");
		remove_fd(fd);
		return false;
	}
	if (ctl == EPOLL_CTL_ADD)
	{
		_clients[fd] = cl;
	}
	return true;
}

void HttpServer::remove_fd(int fd)
{
	if (fd < 0 || _clients.count(fd) == 0)
		return;
	if (epoll_ctl(this->_epoll_fd, EPOLL_CTL_DEL, fd, NULL) == -1)
	{
		perror("epoll_ctl");
	}
	_clients.erase(fd);
	_cgi_to_client.erase(fd);
}

void HttpServer::handle_read(std::shared_ptr<Client> client)
{
	char buffer[READ_BUFFER_SIZE];

	ssize_t bytes_read = read(client->fd, buffer, READ_BUFFER_SIZE);
	if (bytes_read == -1)
	{
		remove_fd(client->fd);
		return;
	}
	if (client->conn_type == CONN_CGI)
	{
		State s = client->req->parse(State::CgiHeader, buffer, bytes_read);

		if (s == State::Ok || s == State::Error)
		{
			finish_cgi_client(client);
			return;
		}
		mod_fd(client->fd, EPOLL_CTL_MOD, EPOLLIN, client);
		return;
	}

	State state = client->req->parse(State::StatusLine, buffer, bytes_read);
	set_config(client, client->req);

	int mask = EPOLLIN;
	if (state == State::Ok || state == State::Error || bytes_read == 0)
	{
		if (bytes_read == 0 && state != State::Ok && state != State::Error)
		{
			remove_fd(client->fd);
			return;
		}
		client->req->dump();
		mask = EPOLLOUT;
		mod_fd(client->fd, EPOLL_CTL_MOD, mask, client);
		return;
	}
	mod_fd(client->fd, EPOLL_CTL_MOD, mask, client);
}

void HttpServer::handle_write(std::shared_ptr<Client> client)
{
	if (client->conn_type == CONN_REGULAR && client->resp == nullptr)
	{
		client->resp = std::make_shared<Response>(client, client->req);
		if (client->conn_type == CONN_WAIT_CGI)
		{
			if (init_cgi_fds(client))
				return;
		}

		client->response = client->resp->buffer.str();
	}
	ssize_t size = client->response.size();
	ssize_t bytes_written = write(client->fd, client->response.data(), size);

	if (bytes_written <= 0)
	{
		remove_fd(client->fd);
		return;
	}
	if (bytes_written < size)
	{
		client->response.erase(0, bytes_written);
		mod_fd(client->fd, EPOLL_CTL_MOD, EPOLLOUT, client);
		return;
	}
	remove_fd(client->fd);
}

void HttpServer::finish_cgi_client(std::shared_ptr<Client> cgi_client)
{
	int conn_fd = _cgi_to_client[cgi_client->fd];
	std::shared_ptr conn = _clients[conn_fd];

	remove_fd(conn->cgi_write_fd);
	remove_fd(conn->cgi_read_fd);

	if (!Cgi::finish(conn->pid))
	{
		std::cerr << "[webserv] CGI error\n";
		conn->resp->set_error(500);
	}
	conn->pid = -1;
	conn->resp->finish_cgi(cgi_client->req);
	conn->response = conn->resp->buffer.str();

	mod_fd(conn->fd, EPOLL_CTL_MOD, EPOLLOUT, conn);
}

bool HttpServer::init_cgi_fds(std::shared_ptr<Client> conn)
{
	_cgi_to_client[conn->cgi_write_fd] = conn->fd;
	_cgi_to_client[conn->cgi_read_fd] = conn->fd;

	std::shared_ptr read_cgi = std::make_shared<Client>(*this, conn->cgi_read_fd);
	std::shared_ptr write_cgi = std::make_shared<Client>(*this, conn->cgi_write_fd);
	write_cgi->response = conn->req->_body;

	int count = mod_fd(conn->cgi_write_fd, EPOLL_CTL_ADD, EPOLLOUT, write_cgi);
	count += mod_fd(conn->cgi_read_fd, EPOLL_CTL_ADD, EPOLLIN, read_cgi);
	if (count != 2)
	{
		remove_fd(conn->cgi_write_fd);
		remove_fd(conn->cgi_read_fd);
		conn->resp->set_error(500);
		return false;
	}
	std::cerr << "[webserv] CGI initialized: " << conn->cgi_read_fd << "/" << conn->cgi_write_fd << std::endl;
	return true;
}

void HttpServer::set_config(std::shared_ptr<Client> client, std::shared_ptr<Request> req)
{
	if (req->host_matched || client->conn_type != CONN_REGULAR) return;

	req->conf = _socketFdToSockets[client->socket]->getServers().front();
	if (req->_headers.count("host") == 0)
		return;
	const std::string host = req->_headers["host"];


	int port = _socketToPort[client->socket];
	std::vector <std::shared_ptr<ServerConfig>> configs = matching_configs(port);

	for (const auto &c : configs)
	{
		for (const auto &name : c->getNames())
		{
			if (host == name)
			{
				std::cerr << "matched host header: " << name << std::endl;
				req->host_matched = true;
				req->conf = c;
				return;
			}
		}
	}
}

std::vector<std::shared_ptr<ServerConfig> > HttpServer::matching_configs(int port)
{
	std::vector<std::shared_ptr<ServerConfig> > CONFIGS;

	for (auto const &x : _portsToSockets)
	{
		std::regex ptrn("(.*):(.*)");
		std::smatch match_res;
		std::regex_match(x.first, match_res, ptrn);
	
		if (_portsToSockets.count(x.first) && port == std::stoi(match_res[2]))
		{
			for (auto con : _portsToSockets[x.first]->getServers())
				CONFIGS.push_back(con);
		}
	}
	return CONFIGS;
}

void HttpServer::close_server(void)
{
	if (this->_epoll_fd == -1)
		return;
	this->_clients.clear();
	this->_socketFdToSockets.clear();
	this->_portsToSockets.clear();
	close(this->_epoll_fd);
	this->_epoll_fd = -1;
}

void HttpServer::close_connections(void)
{
	if (this->_epoll_fd == -1)
		return;
	for (auto &c : _clients)
	{
		c.second->close_fd();
	}
	this->_socketFdToSockets.clear();
	this->_portsToSockets.clear();
	close(this->_epoll_fd);
	this->_epoll_fd = -1;
}
