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
	}
}

bool Cgi::parent_init(int pid, int *fd)
{
	close(fd[1]);
	fd[1] = -1;

	if (dup2(fd[0], STDIN_FILENO) < 0)
	{
		kill(pid, SIGTERM);
		close_pipes(fd);
		return false;
	}
	if (!Io::set_nonblocking(fd[0]))
	{
		kill(pid, SIGTERM);
		close_pipes(fd);
		return false;
	}
	return true;
}

bool Cgi::parent_read(int pid, int *fd, std::string &body)
{
	int	status;
	char	buf[1024];
	ssize_t bytes_read;

	sleep(1);
	bytes_read = read(fd[0], buf, sizeof(buf));

	std::cout << "cgi bytes_read: " << bytes_read << std::endl;
	body += std::string(buf, bytes_read);
	std::cout << "cgi_body: " << body << std::endl;
	/*
	while ((bytes_read = read(fd[0], buf, sizeof(buf))) > 0) {
		body += std::string(buf, bytes_read);
	}
	*/
	close_pipes(fd);
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

	return bytes_read == 0;
}

void Cgi::child_process(int *fd, std::vector <char *> args)
{
	std::vector<char*> c_env;

	for (const auto& var : _env) {
		c_env.push_back(const_cast<char*>(var.c_str()));
	}
	if (::chdir(_script_dir.c_str()) == -1)
	{
		return;
	}
	c_env.push_back(NULL);

	close(fd[0]);
	fd[0] = -1;

	if (dup2(fd[1], STDOUT_FILENO) < 0) {
		return;
	}
	execve(args.data()[0], args.data(), c_env.data());
}

bool Cgi::start(std::string &body)
{
	bool	ret = false;
	int	fd[2];
	int	pid;


	std::vector <char *> args;
	args.push_back(const_cast<char *>(_interpreter.c_str()));
	args.push_back(const_cast<char *>(_script_path.c_str()));
	args.push_back(nullptr);

	if (pipe(fd) == -1)
		return false;
	if ((pid = fork()) == -1)
	{
		close_pipes(fd);
		return false;
	}
	if (pid == 0)
	{
		child_process(fd, args);
		close_pipes(fd);
		exit(1);
	}
	this->_pids.push_back(pid);
	ret = parent_init(pid, fd);
	parent_read(pid, fd, body);
	close_pipes(fd);

	return ret;
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


