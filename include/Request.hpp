#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <string>
#include <vector>
#include <iostream>
#include <map>

#define URI_CHARS "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-._~:/?#[]@!$&'()*+,;="

enum class State
{
	METHOD,
	HEADERS,
	BODY,
	ERROR,
	DONE,
};

enum
{
	PARSER_ERROR,
	PARSER_OK,
};

class Request
{
	protected:
		std::string _method;
		std::string _uri;
		std::string _version;
		std::string _host;
		std::vector<char> _body;
		std::map<std::string, std::string> _headers;
		State parse_method(std::string &line);
		State parse_entry(const std::string &line);
	public:
		Request();
		~Request();
		int parse(std::string &data);
		void	dump(void);
};

#endif
