#include <HttpServer.hpp>

std::unordered_map<std::string, int> method_map = { { "GET", METHOD_GET },
						    { "POST", METHOD_POST },
						    { "DELETE", METHOD_DELETE } };

Request::Request()
{
	this->_bytes_read = 0;
	this->parser_error = 0;
	this->_state = State::StatusLine;
	this->_content_len = 0;
	this->_body_type = BODY_TYPE_NORMAL;
	this->_cgi = false;
	this->conf = nullptr;
	this->_header_delim = CRLF;
	this->_total_read = 0;
	this->host_matched = false;
}

Request::Request(bool cgi)
	: Request()
{
	this->_cgi = cgi;
	this->_header_delim = "\n";
}

Request::~Request()
{
}

State Request::parse(State s_start, char *data, size_t size)
{
	_buffer.append(data, size);
	_bytes_read = size;
	_total_read += size;

	if (_state < s_start)
		_state = s_start;
	while (_state != State::Ok && _state != State::Error)
	{
		switch (_state)
		{
		case State::PartialStatus:
		case State::StatusLine:
			_state = parse_status_line();
			break;
		case State::PartialHeader:
		case State::Header:
			_state = parse_header();
			break;
		case State::PartialBody:
		case State::Body:
			_state = parse_body();
			break;
		case State::CgiHeader:
			_state = parse_header_cgi();
			break;
		case State::PartialCgiBody:
		case State::CgiBody:
			_state = parse_body_cgi();
			break;
		case State::PartialChunked:
		case State::Chunked:
			_state = parse_chunked();
			break;
		case State::MultiPart:
			parse_multipart();
			break;
		default:
			_state = State::Error;
			break;
		}
		if (_state >= State::PartialStatus && _state <= State::PartialBody)
			break;
	}
	if (_state == State::Error && !this->parser_error)
	{
		if (_cgi)
			this->parser_error = 502;
		else
			this->parser_error = STATUS_BAD_REQUEST;
	}
	return _state;
}

State Request::parse_status_line(void)
{
	if (_total_read >= MAX_HEADER_BYTES)
	{
		this->parser_error = STATUS_BAD_REQUEST;
		return State::Error;
	}
	size_t pos = _buffer.find(CRLF);

	if (pos == std::string::npos)
	{
		return State::PartialStatus;
	}
	std::string str = _buffer.substr(0, pos);
	std::regex r(R"((\S+) (\S+) (\S+))");
	std::smatch m;
	if (!std::regex_match(str, m, r))
		return State::Error;
	_method_str = m[1];
	_uri = Str::url_decode(m[2]);
	_version = m[3];

	if (method_map.count(_method_str) == 0)
	{
		this->parser_error = 501;
		return State::Error;
	}
	if (_uri.at(0) != '/' || _version != "HTTP/1.1")
		return State::Error;
	size_t pos_q = _uri.find("?");
	if (pos_q != std::string::npos)
	{
		_query_string = _uri.substr(pos_q + 1, _uri.size());
		_uri.erase(pos_q, _uri.size());
	}
	_method = method_map[_method_str];

	_buffer.erase(0, pos + 2);
	return State::Header;
}

State Request::parse_header(void)
{
	if (_total_read >= MAX_HEADER_BYTES)
	{
		this->parser_error = STATUS_BAD_REQUEST;
		return State::Error;
	}

	size_t pos = _buffer.find(_header_delim);

	if (pos == std::string::npos)
		return State::PartialHeader;
	if (pos == 0)
	{
		_buffer.erase(0, _header_delim.size());
		return State::Body;
	}
	return parse_header_field(pos) ? State::Header : State::Error;
}

State Request::parse_header_cgi(void)
{
	size_t pos = _buffer.find(_header_delim);

	if (pos == std::string::npos)
		return State::CgiBody;
	if (pos == 0)
	{
		_buffer.erase(0, _header_delim.size());
		return State::CgiBody;
	}
	return parse_header_field(pos) ? State::CgiHeader : State::Error;
}

bool Request::parse_header_field(size_t pos)
{
	std::string line = _buffer.substr(0, pos);
	_buffer.erase(0, pos + _header_delim.size());

	std::regex regex(R"((\S+): (.+))");
	std::smatch m;

	if (!std::regex_match(line, m, regex))
		return false;
	std::string key = m[1];
	std::string value = m[2];
	std::transform(key.begin(), key.end(), key.begin(),
		       [](unsigned char c) { return std::tolower(c); });

	_headers[key] = value;
	if (key == "content-length")
	{
		int tmp = Str::content_len_int(value);
		if (tmp < 0 || _method == METHOD_GET)
			return false;
		_content_len = tmp;
	}
	if (key == "transfer-encoding" && value == "chunked")
		_body_type = BODY_TYPE_CHUNKED;
	if (key == "content-type" && value.find("multipart/form-data; boundary=") == 0)
		_body_type = BODY_TYPE_MULTIPART;
	return true;
}

State Request::parse_body(void)
{
	if (_headers.count("host") == 0)
	{
		this->parser_error = STATUS_BAD_REQUEST;
		return State::Error;
	}
	if (conf && _buffer.size() > conf->getMaxSize())
	{
		this->parser_error = STATUS_TOO_LARGE;
		return State::Error;
	}
	if (_body_type == BODY_TYPE_CHUNKED)
		return State::Chunked;
	bool has_length = _headers.count("content-length");
	if (!has_length && _buffer.size() > 0) {
		return State::Error;
	}

	if (has_length && _buffer.size() < _content_len)
		return State::PartialBody;
	if (_body_type == BODY_TYPE_MULTIPART)
		return State::MultiPart;
	_body = _buffer.substr(0, _content_len);
	_buffer.clear();
	return State::Ok;
}

State Request::parse_body_cgi(void)
{
	if (_bytes_read == 0) //eof
		return State::Ok;
	if (_body_type == BODY_TYPE_CHUNKED)
		return State::Chunked;
	if (!_headers.count("content-length"))
	{
		_body += _buffer;
		_buffer.clear();
		return State::PartialCgiBody;
	} else if (_body.size() < _content_len)
	{
		_body += _buffer;
		_buffer.clear();
		return State::PartialCgiBody;
	}
	_buffer.clear();
	return State::Ok;
}

State Request::parse_chunked(void)
{
	if (conf && _buffer.size() > conf->getMaxSize())
	{
		this->parser_error = STATUS_TOO_LARGE;
		return State::Error;
	}
	if (_bytes_read == 0)
		return State::Ok;
	size_t pos = _buffer.find(CRLF);
	if (pos == std::string::npos)
		return State::PartialChunked;

	int num_len = 0;
	int chunk_len = Str::decode_hex(_buffer.c_str(), &num_len);

	if (chunk_len == -1 || num_len == 0)
		return State::Error;
	if (chunk_len == 0)
		return State::Ok;
	std::string chunk = _buffer.substr(pos + 2, chunk_len);
	_body += chunk;
	_buffer.erase(0, pos + 2);
	_buffer.erase(0, chunk.size());
	_buffer.erase(0, 2);

	return State::Chunked;
}

void Request::parse_multipart(void)
{
	std::regex ptrn(".*boundary=(.*)");
	std::smatch match_res;

	if (!std::regex_match(_headers["content-type"], match_res, ptrn))
	{
		this->_state = State::Error;
		return;
	}
	std::string boundary = "--";
	boundary += match_res[1];
	boundary += "\r\n";

	size_t pos = 0, end = 0, header_end = 0;
	while ((pos = _buffer.find(boundary, pos)) != std::string::npos)
	{
		if ((end = _buffer.find(boundary, pos += boundary.size())) ==
		    std::string::npos)
			break;
		size_t buf_size = end - pos;
		if (end - pos >= 2)
			buf_size -= 2;
		std::string part_buf = _buffer.substr(pos, buf_size);
		pos = end;
		if ((header_end = part_buf.find("\r\n\r\n")) == std::string::npos)
			continue;

		struct Part part;
		part.data = part_buf.substr(header_end + 4);
		part_buf.erase(header_end);
		part.name = Str::get_key_data(part_buf, "name");
		part.filename = Str::get_key_data(part_buf, "filename");
		part.content_type = Str::safe_substr(part_buf, "Content-Type: ", CRLF);
		if (part.filename.empty() || part.data.empty())
			continue;

		/*
		std::cout << "part.name: " << part.name << std::endl;
		std::cout << "part.filename: " << part.filename << std::endl;
		std::cout << "part.content_type: " << part.content_type << std::endl;
		std::cout << "part.data.size: " << part.data.size() << std::endl;
		*/
		this->parts.push_back(part);
	}
	_state = State::Ok;
	_buffer.clear();
}

void Request::check_body_limit(void)
{
	if (!this->conf || _headers.count("host") == 0)
		return;
	if (_body.size() > this->conf->getMaxSize())
	{
		this->parser_error = STATUS_TOO_LARGE;
		this->_state = State::Error;
	}
}

void Request::dump(void)
{
	std::cout << "[webserv] " << _method_str << " | ";
	std::cout << _uri << " | " << _query_string << " | ";
	std::cout << _bytes_read << " | " << _headers["host"];
	std::cout << "\n";

	/*
	for(const auto &e : this->_headers)
	{
		std::cout << "\t" << e.first << ": " << e.second << std::endl;
	}
	if (_content_len > 0)
		std::cout << "body: " << _body << std::endl;
	*/
}

bool Request::is_method_allowed(std::vector<std::string> allowed, std::string method)
{
	if (std::find(allowed.begin(), allowed.end(), method) == allowed.end())
	{
		return false;
	}
	return true;
}

