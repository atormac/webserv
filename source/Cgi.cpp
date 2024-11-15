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

bool Cgi::execute(std::shared_ptr<Request> request, std::string &body)
{
	int	fd[2];
	char	**env = NULL;
	int	pid;
	int	status;
	std::string cgi_path = "/usr/bin/php";
	std::string cgi_arg_path = "./www" + request->_uri;
	std::vector <char *> args;


	args.push_back(const_cast<char *>(cgi_path.c_str()));
	args.push_back(const_cast<char *>(cgi_arg_path.c_str()));
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
		std::cout << "execve" << std::endl;
		execve("/usr/bin/php", args.data(), env);
		exit(1);
	}
	std::cout << "waiting for interpreter\n";
	char	buf[1024] = { 0 };
	waitpid(pid, &status, 0);
	close(fd[1]);
	dup2(fd[0], STDIN_FILENO);
	std::cout << "read\n";
	size_t bytes_read = read(fd[0], buf, 1024);
	printf("cgi %zu: %s\n", bytes_read, buf);

	close_pipes(fd);
	if (bytes_read < 0)
		return false;
	body = std::string(buf, bytes_read);
	return true;
}

