#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <string>
#include <vector>
#include <iostream>
#include <map>
#include <algorithm>

#define CRLF "\r\n"
#define URI_CHARS "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-._~:/?#[]@!$&'()*+,;="
#define FIELD_CHARS "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_"

enum class State
{
	StatusLine,
	Header,
	Body,
	PartialStatus,
	PartialHeader,
	PartialBody,
	Complete,
	Error,
};

class Request
{
	private:
		std::string _buffer;

		void parse_status_line(void);
		void parse_header(void);
		bool parse_header_field(size_t pos);
		void	parse_body(void);
	public:
		State _state;
		bool	    _is_chunked;
		std::string _method;
		std::string _uri;
		std::string _version;
		std::string _host;
		std::string _transfer_encoding;
		std::string _body;
		size_t	    _content_len;
		std::map<std::string, std::string> _headers;

		Request();
		~Request();
		State parse(std::string &data);
		void	dump(void);
};

#endif
