#include <HttpServer.hpp>

std::unordered_map<std::string, std::string> cgi_map =
			     {{".php",  "/usr/bin/php"},
			     {".py",  "/usr/bin/python3"}};


Cgi::Cgi() {}

Cgi::Cgi(std::shared_ptr <Location> location, std::shared_ptr<Request> request)
{
	std::string ext = Io::get_file_ext(request->_uri);
	_interpreter = location->_cgi[ext];
	std::filesystem::path p = location->_rootPath;
	p += request->_uri;
	_script_path = p.filename();
	//_script_path = location->_rootPath + request->_uri;

	std::filesystem::path dir = std::filesystem::current_path();
	dir += "/";
	dir += location->_rootPath;
	dir += request->_uri;
	_script_dir = p.parent_path();

	env_set_vars(request);
}

Cgi::~Cgi() {}



void Cgi::env_set(const std::string &key, const std::string &value)
{
	std::string var = key;
	var += "=";
	var += value;
	_env.push_back(var);
}

void Cgi::env_set_vars(std::shared_ptr<Request> request)
{
	env_set("SERVER_NAME", SERVER_NAME);
	env_set("SERVER_PROTOCOL", "HTTP/1.1");
	env_set("GATEWAY_INTERFACE", "CGI/1.1");
	env_set("REQUEST_METHOD", request->_method_str);
	env_set("QUERY_STRING", request->_query_string);
	env_set("PATH_INFO", request->_uri);
	env_set("HTTP_ACCEPT", request->_headers["accept"]);
	env_set("HTTP_USER_AGENT", request->_headers["user-agent"]);

	if (request->_method == METHOD_POST)
	{
		env_set("CONTENT_TYPE", "application/x-www-form-urlencoded");
		env_set("CONTENT_LENGTH", std::to_string(request->_body.size()));
		//std::cout << "request->_body: " << request->_body.data() << std::endl;
	}
}

bool Cgi::parent_init(int pid, int *fd_from, int *fd_to)
{

	/*
	if (dup2(fd[0], STDIN_FILENO) < 0)
	{
		kill(pid, SIGTERM);
		close_pipes(fd);
		return false;
	}
	*/
	if (!Io::set_nonblocking(fd_to[1]) || !Io::set_nonblocking(fd_from[0]))
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

void Cgi::child_process(std::vector <char *> args, int *fd_from, int *fd_to)
{
	std::vector<char*> c_env;

	for (const auto& var : _env) {
		c_env.push_back(const_cast<char*>(var.c_str()));
	}

	if (::chdir(_script_dir.c_str()) == -1) {
		return;
	}

	c_env.push_back(NULL);

	close(fd_to[1]);
	fd_to[1] = -1;
	close(fd_from[0]);
	fd_from[0] = -1;

	if (dup2(fd_to[0], STDIN_FILENO) < 0 || dup2(fd_from[1], STDOUT_FILENO) < 0) {
		return;
	}
	/*
	close(fd_to[0]);
	fd_to[0] = -1;
	close(fd_from[1]);
	fd_from[1] = -1;
	*/
	execve(args.data()[0], args.data(), c_env.data());
}

bool Cgi::start(Client *client)
{
	bool	ret = false;
	int	fd_from[2] = { -1, -1 }; //read cgi output
	int	fd_to[2] = { -1, -1 }; //write cgi input
	int	pid;


	std::vector <char *> args;
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
		child_process(args, fd_from, fd_to);
		close_pipes(fd_from);
		close_pipes(fd_to);
		exit(1);
	}
	this->_pids.push_back(pid);

	ret = parent_init(pid, fd_from, fd_to);
	if (ret)
	{
		client->cgi_from[READ] = fd_from[READ];
		client->cgi_from[WRITE] = fd_from[WRITE];
		client->cgi_to[READ] = fd_to[READ];
		client->cgi_to[WRITE] = fd_to[WRITE];
		client->pid = pid;
	}
	//close_pipes(fd);

	return ret;
}

bool Cgi::finish(int pid, int *fd_from, int *fd_to)
{
	int	status;

	close_pipes(fd_from);
	close_pipes(fd_to);

	if (pid < 0)
		return false;
	if (waitpid(pid, &status, WNOHANG) == -1)
	{
		kill(pid, SIGTERM);
		return false;
	}
	if (status != 0)
	{
		kill(pid, SIGTERM);
		return false;
	}
	std::cout << "CGI::finish success" << std::endl;
	return true;
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

bool Cgi::is_cgi(std::shared_ptr <Location> location, std::string uri)
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


