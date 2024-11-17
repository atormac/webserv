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

bool Cgi::find_cgi(std::string uri)
{
	std::string ext = Io::get_file_ext(uri);
	if (cgi_map.count(ext) == 0)
		return false;
	_cgi = cgi_map[ext];
	_cgi_arg = "./www" + uri;
	std::cout << "cgi_path: " << cgi_map[ext] << std::endl;
	return true;
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
	env_set("HTTP_ACCEPT", request->_headers["accept"]);
	env_set("HTTP_USER_AGENT", request->_headers["user-agent"]);

	if (request->_method == METHOD_POST)
	{
		env_set("CONTENT_TYPE", "application/x-www-form-urlencoded");
		env_set("CONTENT_LENGTH", std::to_string(request->_body.size()));
	}
}

bool Cgi::handle_parent(int *fd, std::string &body)
{
	int	status;
	char	buf[1024];
	ssize_t bytes_read;

	waitpid(-1, &status, 0);
	close(fd[1]);
	fd[1] = -1;
	dup2(fd[0], STDIN_FILENO);
	while ((bytes_read = read(fd[0], buf, sizeof(buf))) > 0) {
		body += std::string(buf, bytes_read);
	}

	return bytes_read == 0;
}

bool Cgi::execute(std::shared_ptr<Request> request, std::string &body)
{
	bool	ret = false;
	int	fd[2];
	int	pid;

	find_cgi(request->_uri);
	env_set_vars(request);

	std::vector <char *> args;
	args.push_back(const_cast<char *>(_cgi.c_str()));
	args.push_back(const_cast<char *>(_cgi_arg.c_str()));
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
		std::vector<char*> envp;
		for (const auto& var : _env)
		{
			envp.push_back(const_cast<char*>(var.c_str()));
		}
		envp.push_back(NULL);

		close(fd[0]);
		dup2(fd[1], STDOUT_FILENO);
		execve(args.data()[0], args.data(), envp.data());
		exit(1);
	}
	ret = handle_parent(fd, body);
	close_pipes(fd);

	return ret;
}

