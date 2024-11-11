#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <string>
#include <vector>
#include <iostream>
#include <map>
#include <algorithm>
#include <Str.hpp>

#define URI_CHARS "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-._~:/?#[]@!$&'()*+,;="
#define FIELD_CHARS "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_"

enum class State
{
	StatusLine,
	Header,
	Body,
	PartialStatus,
	PartialHeader,
	PartialChunked,
	PartialBody,
	Chunked,
	MultiPart,
	Complete,
	Error,
};

struct Part
{
	std::string name;
	std::string filename;
	std::string content_type;
	std::string data;
};

class Request
{
	private:
		std::string _buffer;
		size_t	    _bytes_read;

		void parse_status_line(void);
		void parse_header(void);
		bool parse_header_field(size_t pos);
		void	parse_body(void);
		void	parse_chunked(void);
		void	parse_multipart(void);
		std::string	get_key_data(std::string &buf, std::string key);
		std::string safe_substr(std::string &buf, std::string before, std::string after);
	public:
		State _state;
		int	    _error;
		bool	    _is_chunked;
		int	    _method;
		std::string _method_str;
		std::string _uri;
		std::string _version;
		std::string _host;
		std::string _transfer_encoding;
		std::string _content_type;
		std::string _boundary;
		std::string _body;
		std::vector<Part> parts;
		size_t	    _content_len;
		std::map<std::string, std::string> _headers;

		Request();
		~Request();
		State parse(char *data, size_t size);
		void	dump(void);
};

#endif
