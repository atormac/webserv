#ifndef CGI_HPP
#define CGI_HPP

#include <Request.hpp>
#include <unordered_map>
#include <iostream>
#include <memory>

extern std::unordered_map<std::string, std::string> cgi_map;

class Cgi
{
	public:
		Cgi();
		~Cgi();
		bool execute(std::shared_ptr<Request> request);
};
#endif
