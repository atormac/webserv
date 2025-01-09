#include <HttpServer.hpp>

HttpServer::HttpServer()
{
	this->_epoll_fd = -1;
	this->_client_count = 0;
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

void HttpServer::close_server(void)
{
	if (this->_epoll_fd == -1)
		return;
	this->_clients.clear();
	this->_cgi_to_client.clear();
	for (const auto &e : this->_socketFdToSockets)
		e.second->close_socket();
	for (const auto &pid : this->_pids)
		Cgi::wait_kill(pid);
	close(this->_epoll_fd);
	this->_epoll_fd = -1;
}

bool HttpServer::init()
{
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
		itr->second->setSocketDescriptor(socketFd);
		_socketFdToSockets.insert({ socketFd, itr->second });
	}

	_epoll_fd = epoll_create(512);
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

	for (auto const &cl : _clients)
	{
		if (cl.second->has_timed_out(now))
		{
			remove_fd(cl.first);
		}
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

			if ((e.events & EPOLLERR) || (e.events & EPOLLRDHUP))
			{
				remove_fd(e.data.fd);
				continue;
			}
			if (this->_socketFdToSockets.count(e.data.fd))
			{
				accept_client(e.data.fd);
				continue;
			}
			if (this->_clients.count(e.data.fd) == 0)
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

	if (_clients.size() >= 512)
	{
		std::cout << "Error: too many connections\n";
		return false;
	}
	int client_fd = accept(_socket_fd, (sockaddr *)&peer_addr, &peer_addr_size);
	if (client_fd == -1)
	{
		perror("accept()");
		return false;
	}
	if (!Io::set_nonblocking(client_fd))
	{
		perror("set_nonblocking");
		close(client_fd);
		return false;
	}
	std::shared_ptr cl = std::make_shared<Client>(this, client_fd, _socket_fd,
						      inet_ntoa(peer_addr.sin_addr));

	return add_fd(client_fd, EPOLL_CTL_ADD, EPOLLIN, cl);
}

bool HttpServer::add_fd(int fd, int ctl, int mask, std::shared_ptr<Client> cl)
{
	struct epoll_event ev;

	std::memset(&ev, 0, sizeof(epoll_event));
	ev.events = EPOLLET | mask;
	ev.data.fd = fd;

	if (epoll_ctl(this->_epoll_fd, ctl, fd, &ev) == -1)
	{
		std::cerr << "epoll_ctl accept failed" << std::endl;
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
		std::cerr << "epoll_ctl error fd: " << fd << std::endl;
		perror("remove_fd epoll_ctl");
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
		add_fd(client->fd, EPOLL_CTL_MOD, EPOLLIN, client);
		return;
	}

	State state = client->req->parse(State::StatusLine, buffer, bytes_read);
	set_config(client, client->req);

	int mask = EPOLLIN;
	if (state == State::Ok || state == State::Error || bytes_read == 0)
	{
		mask = EPOLLOUT;
		client->req->dump();
	}
	add_fd(client->fd, EPOLL_CTL_MOD, mask, client);
}

void HttpServer::handle_write(std::shared_ptr<Client> client)
{
	if (client->conn_type == CONN_REGULAR && client->resp == nullptr)
	{
		client->resp = std::make_shared<Response>(client, client->req);
		if (client->conn_type == CONN_WAIT_CGI)
		{
			add_cgi_fds(client);
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
		add_fd(client->fd, EPOLL_CTL_MOD, EPOLLOUT, client);
		return;
	}
	remove_fd(client->fd);
}

void HttpServer::finish_cgi_client(std::shared_ptr<Client> client)
{
	std::shared_ptr ref = client->ref;
	int read_fd = ref->cgi_from[READ];
	int write_fd = ref->cgi_to[WRITE];

	remove_fd(read_fd);
	remove_fd(write_fd);

	if (!Cgi::finish(client->pid, ref->cgi_from, ref->cgi_from))
	{
		std::cerr << "[webserv] CGI error\n";
		ref->resp->set_error(500);
	}
	_pids.erase(client->pid);

	ref->resp->finish_cgi(client->req);
	ref->response = ref->resp->buffer.str();

	add_fd(ref->fd, EPOLL_CTL_MOD, EPOLLOUT, ref);
}

//garbage code
void HttpServer::add_cgi_fds(std::shared_ptr<Client> cl)
{
	int pid = cl->pid;
	int rfd = cl->cgi_from[READ];
	int wfd = cl->cgi_to[WRITE];

	_pids.insert(pid);

	std::cerr << "[webserv] CGI initialized: " << rfd << "/" << wfd << std::endl;

	std::shared_ptr write_cgi = std::make_shared<Client>(this, wfd, pid, cl);
	write_cgi->response = cl->req->_body;

	add_fd(wfd, EPOLL_CTL_ADD, EPOLLOUT, write_cgi);
	_cgi_to_client.emplace(wfd, cl);

	std::shared_ptr read_cgi = std::make_shared<Client>(this, rfd, pid, cl);

	add_fd(rfd, EPOLL_CTL_ADD, EPOLLIN, read_cgi);
	_cgi_to_client.emplace(rfd, cl);
}

void HttpServer::set_config(std::shared_ptr<Client> client, std::shared_ptr<Request> req)
{
	//Fallback to first on the list
	req->conf = _socketFdToSockets[client->socket]->getServers().front();
	if (client->req->_headers.count("host") == 0)
		return;
	const std::string host = client->req->_headers["host"];

	for (const auto &so : _socketFdToSockets)
	{
		for (const auto &server : so.second->getServers())
		{
			for (const auto &name : server->getNames())
			{
				if (name == host)
				{
					req->conf = server;
					return;
				}
			}
		}
	}

	for (const auto &server : _socketFdToSockets[client->socket]->getServers())
	{
		for (const auto &name : server->getNames())
		{
			if (name == host)
			{
				req->conf = server;
				return;
			}
		}
		if (host == server->getIpAddress() + ":" + server->getPort())
		{
			req->conf = server;
			return;
		}
	}
}
