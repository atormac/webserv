#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <HttpServer.hpp>

#define URI_CHARS \
	"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-._~:/?#[]@!$&'()*+,;="
#define FIELD_CHARS "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_"

enum class State {
	StatusLine,
	Header,
	Body,
	CgiHeader,
	CgiBody,
	PartialStatus,
	PartialHeader,
	PartialChunked,
	PartialCgiBody,
	PartialBody,
	Chunked,
	MultiPart,
	Ok,
	Error,
};

struct Part
{
	std::string name;
	std::string filename;
	std::string content_type;
	std::string data;
};

enum {
	BODY_TYPE_NORMAL,
	BODY_TYPE_CHUNKED,
	BODY_TYPE_MULTIPART,
};

class ServerConfig;

class Request {
    private:
	std::string _buffer;
	size_t _bytes_read;
	size_t _total_read;

	bool _cgi;

	std::string _header_delim;

	State parse_header_cgi(void);

	State parse_status_line(void);
	State parse_header(void);
	bool parse_header_field(size_t pos);
	State parse_body(void);
	State parse_body_cgi(void);
	State parse_chunked(void);
	void parse_multipart(void);

    public:
	State _state;
	int parser_error;
	int _method;
	std::string _method_str;
	std::string _uri;
	std::string _version;
	std::string _query_string;
	std::map<std::string, std::string> params;
	std::map<std::string, std::string> _headers;

	bool host_matched;
	std::vector<Part> parts;
	std::shared_ptr<ServerConfig> conf;

	size_t _content_len;
	std::string _body;
	int _body_type;

	Request();
	Request(bool cgi);
	~Request();
	State parse(State s_start, char *data, size_t size);
	void dump();
	static bool is_method_allowed(std::vector<std::string> allowed,
				      std::string method);
};

#endif
