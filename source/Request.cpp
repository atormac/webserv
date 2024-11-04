#include <Request.hpp>
#include <Defines.hpp>
#include <sstream>
#include <unordered_map>

std::unordered_map<std::string, int> method_map =
			        {{ "GET", METHOD_GET },
			        { "POST", METHOD_POST },
			        { "DELETE", METHOD_DELETE }};

Request::Request()
{
	this->_error = 0;
	this->_state = State::StatusLine;
	this->_content_len = 0;
	this->_is_chunked = false;
}

Request::~Request()
{
}

State Request::parse(char *data, size_t size)
{
	_buffer.append(data, size);
	while (_state != State::Complete && _state != State::Error)
	{
		switch (_state)
		{
			case State::PartialStatus:
			case State::StatusLine:
				parse_status_line();
				break;
			case State::PartialHeader:
			case State::Header:
				parse_header();
				break;
			case State::PartialBody:
			case State::Body:
				parse_body();
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


void Request::parse_status_line(void)
{
	_state = State::Error;
	size_t pos = _buffer.find(CRLF);
	if (pos == 0)
		return ;
	if (pos == std::string::npos)
	{
		_state = State::PartialStatus;
		return;
	}
	std::string method_name;
	std::istringstream req_line(_buffer.substr(0, pos));
	_buffer.erase(0, pos + 2);
	req_line >> method_name >> _uri >> _version;

	if (req_line.fail())
		return;
	if (method_map.count(method_name) == 0)
	{
		_error = STATUS_METHOD_NOT_ALLOWED;
		return; 
	}
	std::cout << "method: " << method_name << std::endl;
	_method = method_map[method_name];
	if (_uri.at(0) != '/' || _uri.find_first_not_of(URI_CHARS) != std::string::npos)
		return;
	if (_version != "HTTP/1.1")
		return;
	_state = State::Header;
}

void Request::parse_header(void)
{
	_state = State::Error;
        size_t pos = _buffer.find(CRLF);

	if (pos == 0)
	{
		_buffer.erase(0, 2);
		_state = State::Complete;
		if (_content_len > 0)
			_state = State::Body;
		return;
	}
	if (pos == std::string::npos)
	{
		_state = State::PartialHeader;
		std::cout << "partial header, cur size: " << _buffer.size() << std::endl;
		return;
	}
	if (!parse_header_field(pos))
		return;
	_state = State::Header;
}

bool Request::parse_header_field(size_t pos)
{
	std::string line = _buffer.substr(0, pos);
	_buffer.erase(0, pos + 2);

	size_t sep = line.find(": ");
	if (sep == 0 || sep == std::string::npos)
		return false;
	std::string key = line.substr(0, sep);
	std::transform(key.begin(), key.end(), key.begin(), [](unsigned char c){ return std::tolower(c); });
	if (key.find_first_not_of(FIELD_CHARS) != std::string::npos)
		return false;
	std::string value = line.substr(sep + 2);
	if (value.empty())
		return false;
	this->_headers[key] = value;
	if (key == "content-length")
		_content_len = std::stoi(value);
	if (key == "host")
		_host = value;
	if (key == "transfer-encoding")
		_transfer_encoding = value;
	if (key == "content-type")
	{
		_content_type = value;
		if (_method == METHOD_POST && value.find("multipart/form-data; boundary=") == 0)
		{
			value.erase(0, sizeof("multipart/form-data; boundary=") - 1);
			_boundary = "--" + value;
		}
	}
	return true;
}


void	Request::parse_body(void)
{
	_state = State::Error;
	std::cout << "parse_body\n";
	if (_buffer.size() > REQUEST_BODY_LIMIT)
	{
		_error = STATUS_TOO_LARGE;
		return;
	}
	if (_buffer.size() < _content_len)
	{
		std::cout << "exiting\n";
		_state = State::PartialBody;
		return;
	}
	if (!_boundary.empty()) //multipart
	{
		std::cout << "RAW BODY: " << _buffer << std::endl;
		_state = State::MultiPart;
		return;
	}
	_body = _buffer.substr(0, _content_len);
	_buffer.clear();
	_state = State::Complete;
}

std::string	Request::get_key_data(std::string &buf, std::string key)
{
	size_t pos = buf.find(key + "=\"");
	if (pos == std::string::npos)
		return "";
	pos += key.size() + 2;
	size_t end = buf.find("\"", pos);
	if (end == std::string::npos)
		return "";
	return buf.substr(pos, end - pos);

}

std::string Request::safe_substr(std::string &buf, std::string before, std::string after)
{
	size_t pos = buf.find(before);
	if (pos == std::string::npos)
		return "";
	size_t end = buf.find(after, pos);
	if (end == std::string::npos)
		return buf.substr(pos + before.size());
	return buf.substr(pos, end - pos);
}

void	Request::parse_multipart(void)
{
	_boundary += "\r\n";
	std::cout << "_boundary: " << _boundary << std::endl;
	size_t pos = 0;

	while ((pos = _buffer.find(_boundary, pos)) != std::string::npos)
	{
		struct Part part;

		pos += _boundary.size();
		size_t end = _buffer.find(_boundary, pos);
		if (end == std::string::npos)
			break;
		std::string part_buf = _buffer.substr(pos, (end - pos) - 2); //Extra CRLF ?
		pos = end;
		size_t header_end = part_buf.find("\r\n\r\n");
		if (header_end == std::string::npos)
			continue;

		part.data = part_buf.substr(header_end + 4);	
		part_buf.erase(header_end);
		part.name = get_key_data(part_buf, "name");
		part.filename = get_key_data(part_buf, "filename");
		part.content_type = safe_substr(part_buf, "Content-Type: ", CRLF);
		if (part.data.empty())
			continue;
		std::cout << "part.name: " << part.name << std::endl;
		std::cout << "part.filename: " << part.filename << std::endl;
		std::cout << "part.content_type: " << part.content_type << std::endl;
		std::cout << "part.data.size: " << part.data.size() << std::endl;
		this->parts.push_back(part);
	}
	_state = State::Complete;
	_body.clear();
	_buffer.clear();
	std::cout << "multipart parsed!\n";
}

void	Request::dump(void)
{
	std::cout << "--- Request::dump() ---" << std::endl;
	if (this->_state != State::Complete)
		std::cout << "State:: != Complete\n";
	std::cout << "METHOD: " << _method << std::endl;
	std::cout << "URI: " << _uri << std::endl;
	std::cout << "VERSION: " << _version << std::endl;
	std::cout << "HOST: " << _host << std::endl;
	std::cout << "CONTENT_LEN: " << _content_len << std::endl;
	for(const auto &e : this->_headers)
	{
		std::cout << e.first << ": " << e.second << std::endl;
	}
	if (_content_len > 0)
		std::cout << "body: " << _body << std::endl;
	std::cout << "--- end dump ---" << std::endl;
}
