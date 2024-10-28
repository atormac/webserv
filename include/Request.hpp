#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <string>
#include <vector>
#include <iostream>

struct header_item
{
	std::string key;
	std::string value;
};

class Request
{
	protected:
		std::string _method;
		std::string _uri;
		std::vector<header_item> headers;
		std::vector<char> content;
	public:
		Request();
		~Request();
		int Parse(std::string data);
};

#endif
