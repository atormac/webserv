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
			remove_client(cl.first);
		}
	}
}

void HttpServer::epoll(void)
{
	struct epoll_event events[MAX_EVENTS];

	while (true)
	{
		int nfds = ::epoll_wait(_epoll_fd, events, MAX_EVENTS, -1);

		if (nfds == -1)
			break;

		cull_clients();

		for (int i = 0; i < nfds; i++)
		{
			epoll_event &e = events[i];

			if ((e.events & EPOLLERR) || (e.events & EPOLLRDHUP) || (e.events & EPOLLHUP))
			{
				remove_client(((Client *)e.data.ptr)->fd);
				continue;
			}
			if (this->_socketFdToSockets.count(e.data.fd))
			{
				accept_client(e.data.fd);
				continue;
			}
			if (e.events & EPOLLIN) {
				handle_read(e);
			}
			else if (e.events & EPOLLOUT) {
				handle_write(e);
			}
		}
	}
}

bool HttpServer::accept_client(int _socket_fd)
{
	struct sockaddr_in peer_addr;
	socklen_t peer_addr_size = sizeof(peer_addr);
	struct epoll_event ev;

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
	ev.events = EPOLLIN | EPOLLET;
	ev.data.fd = 0;
	ev.data.ptr = new Client(client_fd, _socket_fd, inet_ntoa(peer_addr.sin_addr));
	Client *client = (Client *)ev.data.ptr;

	if (epoll_ctl(this->_epoll_fd, EPOLL_CTL_ADD, client_fd, &ev) == -1)
	{
		std::cerr << "epoll_ctl accept failed" << std::endl;
		remove_client(client_fd);
		return false;
	}
	_clients.emplace(client_fd, client);
	std::cout << "client added\n";
	return true;
}

void HttpServer::remove_client(int fd)
{
	if (fd < 0)
		return;
	if (epoll_ctl(this->_epoll_fd, EPOLL_CTL_DEL, fd, NULL) == -1)
	{
		perror("remove_client epoll_ctl");
		return;
	}
	close(fd);
	_clients.erase(fd);
}

void HttpServer::handle_read(epoll_event &event)
{
	Client *client = (Client *)event.data.ptr;
	struct epoll_event ev_new;
	char buffer[READ_BUFFER_SIZE];

	ssize_t bytes_read = read(client->fd, buffer, READ_BUFFER_SIZE);
	if (bytes_read == -1)
	{
		remove_client(client->fd);
		return;
	}

	State state = client->req->parse(State::StatusLine, buffer, bytes_read);
	set_config(client, client->req);

	ev_new.events = EPOLLET | EPOLLIN;
	ev_new.data.fd = 0;
	ev_new.data.ptr = event.data.ptr;
	if (state == State::Complete || state == State::Error || bytes_read == 0)
	{
		ev_new.events = EPOLLET | EPOLLOUT;
		client->req->dump();
	}
	if (epoll_ctl(this->_epoll_fd, EPOLL_CTL_MOD, client->fd, &ev_new) == -1)
	{
		perror("epoll_ctl");
		remove_client(client->fd);
	}
}

void HttpServer::handle_write(epoll_event &event)
{
	Client *client = (Client *)event.data.ptr;
	struct epoll_event ev_new;

	std::cout << "[webserv] write " << client->ip_addr << std::endl;
	if (client->response.size() == 0)
	{
		Response resp(client->req);
		client->response = resp.buffer.str(); 
	}

	ssize_t resp_size = client->response.size();
	ssize_t bytes_written = write(client->fd, client->response.data(), client->response.size());
	if (bytes_written <= 0)
	{
		remove_client(client->fd);
		return;
	}
	if (bytes_written < resp_size)
	{
		std::cout << "writing chunk\n";	
		client->response.erase(0, bytes_written);
		ev_new.events = EPOLLET | EPOLLOUT;
		ev_new.data.fd = 0;
		ev_new.data.ptr = event.data.ptr;
		if (epoll_ctl(this->_epoll_fd, EPOLL_CTL_MOD, client->fd, &ev_new) == -1)
		{
			perror("epoll_ctl");
			remove_client(client->fd);
		}
		return;
	}
	remove_client(client->fd);
}

void HttpServer::set_config(Client *client, std::shared_ptr <Request> req)
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
			std::cout << "SERVER:" << name << std::endl;
		}
		if (host == server->getIpAddress() + ":" + server->getPort())
		{
			req->conf = server;
			std::cout << std::endl;
			std::cout << server->getIpAddress() << ":" << server->getPort() << "\n";
			for (const auto &loc : server->getLocations())
			{
				loc->dump();
			}
			return;
		}
	}
}
