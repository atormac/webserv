#include <Request.hpp>
#include <sstream>

Request::Request()
{
}

Request::~Request()
{
}

State Request::parse_entry(const std::string &line)
{
        auto sep = line.find(':');
        if (sep != std::string::npos)
	{
		std::string key = line.substr(0, sep);
		std::string value = line.substr(sep + 1);
		this->_headers[key] = value;
		return State::HEADERS;
        }
	return State::ERROR;
}

State Request::parse_method(std::string &line)
{
	std::istringstream req_line(line);

	req_line >> _method >> _uri >> _version;
	if (req_line.fail())
		return State::ERROR;
	if (_method != "GET" && _method != "POST" && _method != "DELETE")
		return State::ERROR;
	if (_uri.find_first_not_of(URI_CHARS) != std::string::npos)
		return State::ERROR;
	if (_version != "HTTP/1.1")
		return State::ERROR;
	if (line.back() != '\r')
		return State::ERROR;
	return State::HEADERS;
}

void	Request::dump(void)
{
	std::cout << "--- Request::dump ---" << std::endl;
	std::cout << "METHOD: " << _method << std::endl;
	std::cout << "URI: " << _uri << std::endl;
	std::cout << "VERSION: " << _version << std::endl;
	std::cout << "HOST: " << _host << std::endl;
	for(const auto &e : this->_headers)
	{
		std::cout << "KEY: " << e.first;
		std::cout << " VAL: " << e.second;
		std::cout << std::endl;
	}
	std::cout << "--- end dump ---" << std::endl;
}

int Request::parse(std::string &data)
{
	std::istringstream  ss(data);
	std::string line;

	State state = State::METHOD;
	while (std::getline(ss, line))
	{
		switch (state)
		{
			case State::METHOD:
				state = parse_method(line);
				break;
			case State::HEADERS:
				if (line == "\r")
					state = State::BODY;
				else
					state = parse_entry(line);
				break;
			case State::BODY:
				//_body.push
				break;
			case State::DONE:
				std::cout << "STATE_DONE\n" << std::endl;
				_host = this->_headers["Host"];
				return PARSER_OK;
			case State::ERROR:
				std::cout << "PARSER ERROR" << std::endl;
				return PARSER_ERROR;
		}
	}
	return state == State::DONE ? PARSER_OK : PARSER_ERROR;
}
