#include <Cgi.hpp>
#include <Io.hpp>
#include <unordered_map>
#include <iostream>

std::unordered_map<std::string, std::string> cgi_map =
			     {{"php",  "/usr/bin/php"},
			     {"py",  "/usr/bin/python3"}};


Cgi::Cgi()
{
}

Cgi::~Cgi()
{
}

bool Cgi::execute(std::shared_ptr<Request> request)
{
	(void)request;
	return true;
}

