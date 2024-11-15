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
	_cgi_arg = "www" + uri;
	std::cout << "cgi_path: " << cgi_map[ext] << std::endl;
	return true;
}

bool Cgi::execute(std::shared_ptr<Request> request, std::string &body)
{
	int	fd[2];
	char	**env = NULL;
	int	pid;
	int	status;

	find_cgi(request->_uri);

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
		close(fd[0]);
		dup2(fd[1], STDOUT_FILENO);
		execve(args.data()[0], args.data(), env);
		exit(1);
	}
	std::cout << "waiting for interpreter\n";
	char	buf[1024] = { 0 };
	waitpid(pid, &status, 0);
	close(fd[1]);
	dup2(fd[0], STDIN_FILENO);

	ssize_t bytes_read;
	while ((bytes_read = read(fd[0], buf, sizeof(buf))) > 0)
	{
		body += std::string(buf, bytes_read);
	}
	std::cout << "cgi_output: " << body << std::endl;

	close_pipes(fd);
	return true;
}

