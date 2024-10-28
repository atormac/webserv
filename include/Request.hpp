#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <string>
#include <vector>
#include <iostream>

enum class State
{
	METHOD,
	HEADERS,
	ERROR,
	DONE,
};

enum
{
	PARSER_ERROR,
	PARSER_OK,
};

struct header_entry
{
	std::string key;
	std::string value;
};

class Request
{
	protected:
		std::string _method;
		std::string _uri;
		std::string _version;
		std::vector<header_entry> _headers;
		std::vector<char> _content;
		State parse_method(std::string &line);
		State parse_entry(const std::string &line);
	public:
		Request();
		~Request();
		int parse(std::string &data);
		void	dump(void);
};

#endif
