#include <HttpServer.hpp>

HttpServer::~HttpServer() {}

void HttpServer::close_server(void)
{
	if (this->_epoll_fd == -1)
		return;
	for(const auto& e : this->_socketFdToSockets)
	{
		e.second->close_socket();
	}
	close(this->_epoll_fd);
	this->_epoll_fd = -1;
}

bool HttpServer::init()
{
	this->_client_count = 0;
	for (std::map<std::string, std::shared_ptr<Socket>>::iterator itr = _portsToSockets.begin(); itr != _portsToSockets.end(); itr++)
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
		_socketFdToSockets.insert({socketFd, itr->second});
	}

	_epoll_fd = epoll_create(512);
	if (_epoll_fd == -1)
	{
		perror("epoll_create");
		return false;
	}
	for(const auto &so : _socketFdToSockets)
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
	for (auto const &cl : _clients) {
	
		if (cl.first == 0 || cl.second == nullptr)
			continue;
		time_t delta = now - cl.second->start_time;
	
		if (delta > 60) {
			std::cout << "client timeout: " << cl.first << std::endl;
			remove_fd(cl.first);
		}
	}
}

void HttpServer::epoll(void)
{
	struct epoll_event events[MAX_EVENTS];

	while (true)
	{
		std::cout << "[webserv] epoll_wait()\n";
		int nfds = ::epoll_wait(_epoll_fd, events, MAX_EVENTS, -1);

		if (nfds == -1)
			break;

		//cull_clients();

		for (int i = 0; i < nfds; i++)
		{
			epoll_event &e = events[i];

			if ((e.events & EPOLLERR) || (e.events & EPOLLRDHUP))
			{
				remove_fd(((Client *)e.data.ptr)->fd);
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
			//on pipe close EPOLLHUP means EOF here
			//mark the event as OK to catch errors directly from read/write
			//calls
			if (e.events & EPOLLHUP) 
			{
				e.events |= EPOLLIN;
				std::cerr << "EOF: " << cl->fd << " | " << cl->conn_type << "\n";
				
			}

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

	if (_clients.size() > 512)
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
	std::shared_ptr cl = std::make_shared<Client>(client_fd, _socket_fd, inet_ntoa(peer_addr.sin_addr));

	std::cout << "client added\n";
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
	_clients[fd] = cl;
	if (mask == EPOLL_CTL_ADD)
		std::cout << "[webserv] fd: " << fd << " added"<< std::endl;
	return true;
}

void HttpServer::remove_fd(int fd)
{
	if (fd < 0 || _clients.count(fd) == 0)
		return;
	if (epoll_ctl(this->_epoll_fd, EPOLL_CTL_DEL, fd, NULL) == -1) {
		perror("remove_fd epoll_ctl");
	}
	close(fd);
	_clients.erase(fd);
	_cgi_to_client.erase(fd);
	std::cout << "[webserv] fd: " << fd << " removed "<< std::endl;
}

void HttpServer::handle_read(std::shared_ptr <Client> client)
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
		std::cout << "bytes_read: " << bytes_read << " | " << client->fd << std::endl;
		if (bytes_read == 0)
		{
			finish_cgi_client(client);
			return;
		}
		client->resp->_body << std::string(buffer, bytes_read);
		add_fd(client->fd, EPOLL_CTL_MOD, EPOLLIN, client);
		return;
	}

	State state = client->req->parse(State::StatusLine, buffer, bytes_read);
	set_config(client, client->req);

	int mask = EPOLLIN;
	std::cout << "adding write epoll: " << client->fd << std::endl;

	if (state == State::Ok || state == State::Error || bytes_read == 0)
	{
		mask = EPOLLOUT;
		client->req->dump();
	}
	add_fd(client->fd, EPOLL_CTL_MOD, mask, client);
}

void HttpServer::finish_cgi_client(std::shared_ptr <Client> client)
{
	int read_fd = client->ref->cgi_from[READ];
	int write_fd = client->ref->cgi_to[WRITE];

	Cgi::finish(client->pid, client->ref->cgi_from, client->ref->cgi_from);
	
	client->resp->finish_response();
	client->ref->response = client->resp->buffer.str();

	add_fd(client->ref->fd, EPOLL_CTL_MOD, EPOLLOUT, client->ref);
	remove_fd(client->fd);
	remove_fd(read_fd);
	remove_fd(write_fd);
}

//garbage code
void	HttpServer::add_cgi_fds(std::shared_ptr <Client> current)
{
	int pid_cgi = current->pid;


	std::cout << __FUNCTION__ << ": cgi read fd: " << current->cgi_from[READ] << std::endl;
	std::cout << __FUNCTION__ << ": cgi write fd: " << current->cgi_to[WRITE] << std::endl;
	if (current->req->_body.size() > 0)
	{
		std::shared_ptr write_cgi = std::make_shared<Client>(current->cgi_to[WRITE], pid_cgi, current);
		write_cgi->response = current->req->_body;

		add_fd(current->cgi_to[WRITE], EPOLL_CTL_ADD, EPOLLOUT, write_cgi);
	}

	std::shared_ptr read_cgi = std::make_shared<Client>(current->cgi_from[READ], pid_cgi, current);
	read_cgi->resp = current->resp;

	add_fd(current->cgi_from[READ], EPOLL_CTL_ADD, EPOLLIN, read_cgi);
	_cgi_to_client.emplace(current->cgi_from[READ], current);
	_cgi_to_client.emplace(current->cgi_to[WRITE], current);
}

void HttpServer::handle_write(std::shared_ptr <Client> client)
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

	ssize_t resp_size = client->response.size();
	ssize_t bytes_written = write(client->fd, client->response.data(), client->response.size());

	std::cout << "[webserv] write " << client->fd << " | "<< client->conn_type << " | " << bytes_written << " | " << client->response.size() << std::endl;

	if (bytes_written <= 0)
	{
		remove_fd(client->fd);
		return;
	}
	if (bytes_written < resp_size)
	{
		client->response.erase(0, bytes_written);
		add_fd(client->fd, EPOLL_CTL_MOD, EPOLLOUT, client);
		return;
	}
	remove_fd(client->fd);
}

void HttpServer::set_config(std::shared_ptr <Client> client, std::shared_ptr <Request> req)
{

	/*
	for(const auto &so : _socketFdToSockets)
	{
		for (const auto &server : so.second->getServers())
		{
			for (const auto &name : server->getNames()) {

				if (name == host)
				{
					req->conf = server;
					return;
				}
			}

		}

	}
	*/
	//Fallback to first on the list
	req->conf = _socketFdToSockets[client->socket]->getServers().front();
	if (client->req->_headers.count("host") == 0)
		return;
	const std::string host = client->req->_headers["host"];
	std::cout << "client host: " << host << std::endl;

	for(const auto &server : _socketFdToSockets[client->socket]->getServers())
	{
		for (const auto &name : server->getNames()) {
			if (name == host) {
				req->conf = server;
				return;
			}
			//std::cout << "SERVER:" << name << std::endl;
		}
		if (host == server->getIpAddress() + ":" + server->getPort())
		{
			req->conf = server;
			std::cout << std::endl;
			std::cout << server->getIpAddress() << ":" << server->getPort() << "\n";
			for (const auto &loc : server->getLocations())
			{
				//loc->dump();
				(void)loc;
			}
			return;
		}
	}
}
