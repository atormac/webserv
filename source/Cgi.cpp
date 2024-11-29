#include <Cgi.hpp>
#include <Io.hpp>
#include <unordered_map>
#include <iostream>
#include <stdlib.h>
#include <stddef.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <vector>
#include <string>
#include <stdio.h>

std::unordered_map<std::string, std::string> cgi_map =
			     {{".php",  "/usr/bin/php"},
			     {".py",  "/usr/bin/python3"}};


Cgi::Cgi()
{
}

Cgi::Cgi(std::shared_ptr<Request> request)
{
	std::string ext = Io::get_file_ext(request->_uri);
	_interpreter = cgi_map[ext];
	_script_path = "./www" + request->_uri;
	env_set_vars(request);
}

Cgi::~Cgi()
{
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
	env_set("HTTP_ACCEPT", request->_headers["accept"]);
	env_set("HTTP_USER_AGENT", request->_headers["user-agent"]);

	if (request->_method == METHOD_POST)
	{
		env_set("CONTENT_TYPE", "application/x-www-form-urlencoded");
		env_set("CONTENT_LENGTH", std::to_string(request->_body.size()));
	}
}

bool Cgi::handle_parent(int pid, int *fd, std::string &body)
{
	int	status;
	char	buf[1024];
	ssize_t bytes_read;

	waitpid(pid, &status, 0);
	if (status != 0)
	{
		close_pipes(fd);
		kill(pid, SIGINT);
	}
	close(fd[1]);
	fd[1] = -1;

	if (dup2(fd[0], STDIN_FILENO) < 0)
		return false;
	while ((bytes_read = read(fd[0], buf, sizeof(buf))) > 0) {
		body += std::string(buf, bytes_read);
	}

	return bytes_read == 0;
}

void Cgi::handle_child(int *fd, std::vector <char *> args)
{
	std::vector<char*> c_env;

	for (const auto& var : _env) {
		c_env.push_back(const_cast<char*>(var.c_str()));
	}
	c_env.push_back(NULL);

	close(fd[0]);
	if (dup2(fd[1], STDOUT_FILENO) < 0) {
		return;
	}
	execve(args.data()[0], args.data(), c_env.data());
}

bool Cgi::execute(std::string &body)
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
	this->_pids.push_back(pid);
	if (pid == 0)
	{
		handle_child(fd, args);
		exit(1);
	}
	ret = handle_parent(pid, fd, body);
	close_pipes(fd);

	return ret;
}

bool Cgi::is_cgi(std::string uri)
{
	if (uri.rfind("/cgi-bin/", 0) != 0)
		return false;
	std::string ext = Io::get_file_ext(uri);
	if (cgi_map.count(ext) == 0)
		return false;
	int flags = Io::file_stat("./www" + uri);
	if (!(flags & FS_ISFILE) || !(flags & FS_READ))
		return false;
	return true;
}
