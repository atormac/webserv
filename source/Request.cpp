#include <Request.hpp>
#include <HttpServer.hpp>
#include <Defines.hpp>
#include <regex>
#include <sstream>
#include <unordered_map>
#include <Str.hpp>

std::unordered_map<std::string, int> method_map =
			        {{ "GET", METHOD_GET },
			        { "POST", METHOD_POST },
			        { "DELETE", METHOD_DELETE }};

Request::Request()
{
	this->_bytes_read = 0;
	this->_error = 0;
	this->_state = State::StatusLine;
	this->_content_len = 0;
	this->_body_type = BODY_TYPE_NORMAL;
	this->conf = nullptr;
}

Request::~Request()
{
}

State Request::parse(State s_start, char *data, size_t size)
{
	_buffer.append(data, size);
	_bytes_read += size;

	if (_state < s_start)
		_state = s_start;
	while (_state != State::Complete && _state != State::Error)
	{
		if (size == 0) {
			std::cout << "unexpected EOF!" << std::endl;
		}
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
			case State::PartialChunked:
			case State::Chunked:
				parse_chunked();
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
	if (_state == State::Error && !_error)
		_error = STATUS_BAD_REQUEST;
	return _state;
}


State Request::parse_status_line(void)
{
	size_t pos = _buffer.find(CRLF);

	if (pos == std::string::npos)
	{
		if (_buffer.rfind("GET ") != 0 && _buffer.rfind("POST ") != 0 && _buffer.rfind("DELETE") != 0)
			return State::Error;
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

	if (method_map.count(_method_str) == 0) {
		_error = STATUS_METHOD_NOT_ALLOWED;
		return State::Error;
	}
	if (_uri.at(0) != '/' || _version != "HTTP/1.1")
		return State::Error;
	size_t pos_q = _uri.find("?");
	if (pos_q != std::string::npos) {

		_query_string = _uri.substr(pos_q + 1, _uri.size());
		_uri.erase(pos_q, _uri.size());
	}
	_method = method_map[_method_str];

	_buffer.erase(0, pos + 2);
	return State::Header;
}

State Request::parse_header(void)
{
        size_t pos = _buffer.find(CRLF);

	if (pos == std::string::npos)
		return State::PartialHeader;
	if (pos == 0)
	{
		_buffer.erase(0, 2);
		if (_body_type == BODY_TYPE_CHUNKED)
			return State::Chunked;
		if (_content_len > 0)
			return State::Body;
		return State::Complete;
	}
	return parse_header_field(pos) ? State::Header : State::Error;
}

bool Request::parse_header_field(size_t pos)
{
	std::string line = _buffer.substr(0, pos);
	_buffer.erase(0, pos + 2);

	std::regex regex(R"((\S+): (.+))");
	std::smatch m;

	if (!std::regex_match(line, m, regex))
		return false;
	std::string key = m[1];
	std::string value = m[2];
	std::transform(key.begin(), key.end(), key.begin(), [](unsigned char c){ return std::tolower(c); });

	_headers[key] = value;
	if (key == "content-length") _content_len = std::stoi(value);
	if (key == "transfer-encoding" && value == "chunked")
		_body_type = BODY_TYPE_CHUNKED;
	if (key == "content-type" && value.find("multipart/form-data; boundary=") == 0)
		_body_type = BODY_TYPE_MULTIPART;

	return true;
}

State	Request::parse_body(void)
{
	if (_method != METHOD_POST)
		return State::Complete;
	if (conf && _buffer.size() > conf->getMaxSize())
	{
		_error = STATUS_TOO_LARGE;
		return State::Error;
	}
	if (_buffer.size() < _content_len)
		return State::PartialBody;
	if (_body_type == BODY_TYPE_MULTIPART)
		return  State::MultiPart;
	_body = _buffer.substr(0, _content_len);
	_buffer.clear();
	std::cout << "body: " << _body << std::endl;
	return State::Complete;
}

void	Request::parse_chunked(void)
{
	std::cout << "parse_chunked\n";
	size_t pos = _buffer.find(CRLF);
	if (pos == std::string::npos)
	{
		_state = State::PartialChunked;
		return;
	}
	int chunk_len = Str::decode_hex(_buffer.c_str());
	if (chunk_len == -1)
	{
		_state = State::Error;
		return;
	}
	if  (chunk_len == 0)
	{
		std::cout << "chunked body: " << _body << std::endl;
		_state = State::Complete;
		return;
	}
	_body += _buffer.substr(pos + 2, chunk_len);
	_buffer.erase(0, pos + 2 + chunk_len + 1);
	if (_buffer.size() > 0)
		parse_chunked();
}

void	Request::parse_multipart(void)
{
	std::string boundary = "--";
	boundary += Str::trim_start(_headers["content-type"], "boundary=");
	boundary += "\r\n";

	size_t pos = 0;
	while ((pos = _buffer.find(boundary, pos)) != std::string::npos)
	{
		pos += boundary.size();
		size_t end = _buffer.find(boundary, pos);
		if (end == std::string::npos)
			break;
		std::string part_buf = _buffer.substr(pos, (end - pos) - 2); //Extra CRLF ?
		pos = end;
		size_t header_end = part_buf.find("\r\n\r\n");
		if (header_end == std::string::npos)
			continue;

		struct Part part;
		part.data = part_buf.substr(header_end + 4);	
		part_buf.erase(header_end);
		part.name = Str::get_key_data(part_buf, "name");
		part.filename = Str::get_key_data(part_buf, "filename");
		part.content_type = Str::safe_substr(part_buf, "Content-Type: ", CRLF);
		if (part.data.empty())
			continue;

		std::cout << "part.name: " << part.name << std::endl;
		std::cout << "part.filename: " << part.filename << std::endl;
		std::cout << "part.content_type: " << part.content_type << std::endl;
		std::cout << "part.data.size: " << part.data.size() << std::endl;
		this->parts.push_back(part);
	}
	std::cout << "MULTIPART REQUEST:\n" << _buffer << std::endl;
	_state = State::Complete;
	_body.clear();
	_buffer.clear();
	std::cout << "multipart parsed.\n";
}

void	Request::dump(void)
{
	std::cout << "[webserv] " << _method_str << " | ";
	std::cout << _uri << " | " << _query_string << " | ";
	std::cout << _bytes_read << " | " << _headers["host"];
	std::cout << std::endl;

	for(const auto &e : this->_headers)
	{
		std::cout << "\t" << e.first << ": " << e.second << std::endl;
	}
	if (_content_len > 0)
		std::cout << "body: " << _body << std::endl;
}
