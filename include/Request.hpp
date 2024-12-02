#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <string>
#include <vector>
#include <iostream>
#include <map>
#include <algorithm>
#include <Str.hpp>
#include <memory>

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

enum
{
	BODY_TYPE_NORMAL,
	BODY_TYPE_CHUNKED,
	BODY_TYPE_MULTIPART,
};

class ServerConfig;

class Request
{
	private:
		std::string _buffer;
		size_t	    _bytes_read;
		size_t	    _bytes_added;

		int	    _body_type;
	
		State parse_status_line(void);
		State parse_header(void);
		bool parse_header_field(size_t pos);
		State	parse_body(void);
		void	parse_chunked(void);
		void	parse_multipart(void);
		std::string	get_key_data(std::string &buf, std::string key);
		std::string safe_substr(std::string &buf, std::string before, std::string after);
	public:
		State _state;
		int	    _error;
		int	    _method;
		std::string _method_str;
		std::string _uri;
		std::string _version;
		std::string _query_string;
		std::map<std::string, std::string> params;
		std::map<std::string, std::string> _headers;

		std::vector<Part> parts; //multipart
		std::shared_ptr <ServerConfig> conf;


		size_t		_content_len;
		std::string	_body;

		Request();
		~Request();
		State parse(State s_start, char *data, size_t size);
		void	dump(void);
};

#endif
