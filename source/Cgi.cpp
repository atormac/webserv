#include <HttpServer.hpp>

std::unordered_map<std::string, std::string> cgi_map = { { ".php", "/usr/bin/php" },
							 { ".py", "/usr/bin/python3" } };

Cgi::Cgi()
{
}

Cgi::Cgi(std::shared_ptr<Location> location, std::shared_ptr<Request> request)
{
	std::string ext = Io::get_file_ext(request->_uri);
	_interpreter = location->_cgi[ext];
	std::filesystem::path p = location->_rootPath;
	p += request->_uri;

	_script_abs = std::filesystem::absolute(p);
	_script_path = p.filename();

	std::filesystem::path dir = std::filesystem::current_path();
	dir += "/";
	dir += location->_rootPath;
	dir += request->_uri;
	_script_dir = p.parent_path();

	env_set_vars(request);
}

Cgi::~Cgi()
{
}

void Cgi::env_set(const std::string &key, const std::string &value)
{
	std::string var = key;
	var += "=";
	var += value;
	_env.push_back(var);
}

void Cgi::env_set_vars(std::shared_ptr<Request> request)
{
	env_set("GATEWAY_INTERFACE", "CGI/1.1");
	env_set("SERVER_PROTOCOL", "HTTP/1.1");
	env_set("REQUEST_METHOD", request->_method_str);
	env_set("QUERY_STRING", request->_query_string);
	env_set("SCRIPT_FILENAME", _script_abs);

	env_set("SERVER_NAME", request->_headers["host"]);
	env_set("HTTP_ACCEPT", request->_headers["accept"]);
	env_set("HTTP_USER_AGENT", request->_headers["user-agent"]);
	env_set("PATH_INFO", request->_uri);

	if (request->_method == METHOD_POST)
	{
		env_set("CONTENT_TYPE", "application/x-www-form-urlencoded");
		env_set("CONTENT_LENGTH", std::to_string(request->_body.size()));
	}
}

bool Cgi::parent_init(int pid, int *fd_from, int *fd_to)
{
	if (!Io::set_nonblocking(fd_to[WRITE]) || !Io::set_nonblocking(fd_from[READ]))
	{
		kill(pid, SIGTERM);
		close_pipes(fd_from);
		close_pipes(fd_to);
		return false;
	}
	close(fd_to[0]);
	fd_to[0] = -1;
	close(fd_from[1]);
	fd_from[1] = -1;
	return true;
}

void Cgi::child_process(std::shared_ptr<Client> client, std::vector<char *> args,
			int *fd_from, int *fd_to)
{
	signal(SIGPIPE, SIG_IGN);
	signal(SIGINT, SIG_IGN);
	std::vector<char *> c_env;

	c_env.reserve(_env.size() + 1);
	for (const auto &var : _env)
	{
		c_env.push_back(const_cast<char *>(var.c_str()));
	}
	c_env.push_back(nullptr);

	if (::chdir(_script_dir.c_str()) == -1)
	{
		return;
	}

	close(fd_to[1]);
	close(fd_from[0]);

	if (dup2(fd_to[0], STDIN_FILENO) < 0 || dup2(fd_from[1], STDOUT_FILENO) < 0)
	{
		return;
	}
	close(fd_to[0]);
	close(fd_from[1]);
	client->cleanup_child();
	execve(args.data()[0], args.data(), c_env.data());
}

bool Cgi::start(std::shared_ptr<Client> client)
{
	bool ret = false;
	int fd_from[2] = { -1, -1 }; //read cgi output
	int fd_to[2] = { -1, -1 }; //write cgi input
	int pid;

	std::vector<char *> args;
	args.push_back(const_cast<char *>(_interpreter.c_str()));
	args.push_back(const_cast<char *>(_script_path.c_str()));
	args.push_back(nullptr);

	if (pipe(fd_from) == -1 || pipe(fd_to) == -1)
	{
		close_pipes(fd_from);
		close_pipes(fd_to);
		return false;
	}
	if ((pid = fork()) == -1)
	{
		close_pipes(fd_from);
		close_pipes(fd_to);
		return false;
	}
	if (pid == 0)
	{
		child_process(client, args, fd_from, fd_to);
		close_pipes(fd_from);
		close_pipes(fd_to);
		exit(1);
	}

	ret = parent_init(pid, fd_from, fd_to);
	if (ret)
	{
		client->cgi_read_fd = fd_from[READ];
		client->cgi_write_fd = fd_to[WRITE];
		client->pid = pid;
	}
	return ret;
}

bool Cgi::finish(int pid)
{
	int status = 0;

	if (waitpid(pid, &status, WNOHANG) == -1)
	{
		kill(pid, SIGTERM);
		return false;
	}
	if (WIFSIGNALED(status))
	{
		kill(pid, SIGTERM);
		return false;
	}
	if (WIFEXITED(status))
	{
		return true;
	}
	kill(pid, SIGTERM);
	return false;
}

void Cgi::close_pipes(int *fd)
{
	if (fd[0] >= 0)
	{
		close(fd[0]);
		fd[0] = -1;
	}
	if (fd[1] >= 0)
	{
		close(fd[1]);
		fd[1] = -1;
	}
}

bool Cgi::is_cgi(std::shared_ptr<Location> location, std::string uri)
{
	if (!location)
		return false;

	std::string cgi_uri = location->_rootPath + uri;
	std::string ext = Io::get_file_ext(cgi_uri);
	if (location->_cgi.count(ext) == 0)
		return false;
	int flags = Io::file_stat(cgi_uri);
	if (!(flags & FS_ISFILE) || !(flags & FS_READ))
		return false;
	return true;
}
