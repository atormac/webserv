#include <Request.hpp>
#include <sstream>

Request::Request()
{
	this->_state = State::StatusLine;
	this->_content_len = 0;
	this->_is_chunked = false;
}

Request::~Request()
{
}

State Request::parse(std::string &data)
{
	_buffer += data;
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
			default:
				_state = State::Error;
				break;
		}
		if (_state >= State::PartialStatus && _state <= State::PartialBody)
			break;
	}
	return _state;
}


void Request::parse_status_line(void)
{
	_state = State::Error;
	size_t pos = _buffer.find("\r\n");
	if (pos == 0)
		return ;
	if (pos == std::string::npos)
	{
		_state = State::PartialStatus;
		return;
	}

	std::istringstream req_line(_buffer.substr(0, pos));
	_buffer.erase(0, pos + 2);
	req_line >> _method >> _uri >> _version;
	if (req_line.fail())
		return;
	if (_method != "GET" && _method != "POST" && _method != "DELETE")
		return; 
	if (_uri.at(0) != '/')
		return;
	if (_uri.find_first_not_of(URI_CHARS) != std::string::npos)
		return;
	if (_version != "HTTP/1.1")
		return;
	_state = State::Header;
}

void Request::parse_header(void)
{
	_state = State::Error;
        size_t pos = _buffer.find("\r\n");

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
	if (key.find_first_not_of(FIELD_CHARS) != std::string::npos)
		return false;
	std::string value = line.substr(sep + 2);
	if (value.empty())
		return false;
	this->_headers[key] = value;
	if (key == "Content-Length")
		_content_len = std::stoi(value);
	if (key == "Host")
		_host = value;
	if (key == "Transfer-Encoding")
		_transfer_encoding = value;
	return true;
}

void	Request::parse_body(void)
{
	_state = State::Error;
	if (_buffer.size() < _content_len)
	{
		_state = State::PartialBody;
		return;
	}
	_body = _buffer.substr(0, _content_len);
	_buffer.erase(0, _content_len);
	_state = State::Complete;
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
