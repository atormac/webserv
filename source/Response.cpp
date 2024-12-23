#include <HttpServer.hpp>

Response::~Response() {}

Response::Response(Client *client, std::shared_ptr<Request> request): _request(request), _status_code(STATUS_NOT_FOUND)
{
	int error_code = this->has_errors();

	if (error_code) {
		set_error_page(error_code);
		create_response(error_code);
		return;
	}

	// Cookie Monster lives here:
	_handleCookies();

	if (!_location->_redirectPath.empty()) {
		create_response(_location->_redirectCode);
		return;
	}

	if (Cgi::is_cgi(_location, _request->_uri)) {

		if (init_cgi(client)) {
			this->_status_code = 200;
			return;
		}
	}
	else 
	{
		switch (_request->_method) {
			case METHOD_GET: handle_get(); break;
			case METHOD_POST: handle_post(); break;
			case METHOD_DELETE: handle_delete(); break;
		}
	}
	finish_response();
}

void Response::finish_response(void)
{
	//std::cout << "FINISH_RESPONSE BODY: \n" << _body.str() << std::endl;
	set_error_page(_status_code);
	create_response(_status_code);
}

void Response::finish_with_body(std::string body)
{
	_body << body;
	set_error_page(_status_code);
	create_response(_status_code);
}

bool Response::init_cgi(Client *client)
{	
	if (_request->_method == METHOD_DELETE) {
		_status_code = STATUS_METHOD_NOT_ALLOWED;
		return false;
	}
	Cgi cgi(_location, _request);

	if (!cgi.start(client))
	{
		_status_code = 404;
		std::cout << "cgi.start failed\n";
		return false;
	}
	client->conn_type = CONN_WAIT_CGI;
	return true;
}

int	Response::has_errors(void)
{
	if (_request->parser_error) {
		return _request->parser_error;
	}

	_location = find_location();
	if (_location == nullptr) {
		return STATUS_NOT_FOUND;
	}

	if (!Request::is_method_allowed(_location->_methods, _request->_method_str)) {
		return STATUS_METHOD_NOT_ALLOWED;
	}
	return 0;
}

void	Response::create_response(int status)
{
	const std::string &bs = _body.str();

	buffer << "HTTP/1.1 " << status << " " << code_map[status] << CRLF;
	buffer << "Content-Length: " << bs.size() << CRLF;
	buffer << "Connection: close" << CRLF;
	buffer << "Date: " << Str::date_str_now() << CRLF;
	buffer << "Server: " << SERVER_NAME << CRLF;
	if (_location && !_location->_redirectPath.empty()) {
		buffer << "Location: " << _location->_redirectPath << CRLF;
	}

	if (bs.size() > 0)
		buffer << "Content-Type: " << get_content_type(_request->_uri) << CRLF;
	if (!_setCookie.empty())
		buffer << "Set-Cookie: " << _setCookie << CRLF;
	buffer << CRLF;
	buffer << bs;
}

void	Response::handle_get(void)
{
	std::string filename = _location->_rootPath + _request->_uri;
	int flags = Io::file_stat(filename);

	if (!flags || !(flags & FS_READ)) {
		_status_code = STATUS_NOT_FOUND;
		return;
	}

	if (flags & FS_ISFILE) {
		if (Io::read_file(filename, _body))
			_status_code = STATUS_OK;
	}
	else if (flags & FS_ISDIR) {
		if (_request->_uri.back() != '/' || !_location->_autoIndex)
			return;
		if (directory_index(filename))
			_status_code = STATUS_OK;
	}
}

void	Response::handle_post(void)
{
	for (auto & part : _request->parts)
	{
		if (Io::write_file(_location->_uploadPath + "/" + part.filename, part.data))
			_status_code = STATUS_OK;
	}
}

void	Response::handle_delete(void)
{
	std::string filename = _location->_rootPath + _request->_uri;
	int flags = Io::file_stat(filename);
	if (!flags) {
		_status_code = STATUS_NOT_FOUND;
		return;
	}
	if (!std::filesystem::remove(filename.c_str())) {
		_status_code = 500;
		return;
	}
	_status_code = STATUS_OK;
}

std::shared_ptr <Location> Response::find_location(void)
{
	std::shared_ptr <Location> ret;

	if (_request->conf == nullptr)
		return nullptr;
	for (const auto &loc : _request->conf->getLocations())
	{
		if (_request->_uri == loc->_path) {
			ret = loc;
			std::cout << "Loc->_path found: " << loc->_path << std::endl;
			break;
		}
		if (_request->_uri.rfind(loc->_path, 0) == 0 && loc->_path.back() == '/') {

			if (!ret || loc->_path.size() > ret->_path.size()) {
				ret = loc;
			}
		}
	}
	return ret;
}

void Response::set_error_page(int code)
{
	std::string page_path = "";

	if (!_request->conf || _request->conf->_errorPages.count(code) == 0)
		return;
	if (_request->conf && _request->conf->_errorPages.count(code)) {
		page_path = _request->conf->_errorPages[code];
		int flags = Io::file_stat(_request->conf->_errorPages[code]);
		if (!flags || !(flags & FS_READ))
			page_path = "";
	}

	if (page_path == "" && code == 404) {
		this->_status_code = code;
		_body << DEFAULT_404;
		return;

	}

	if (!Io::read_file(page_path, _body))
		this->_status_code = 500;
}

bool	Response::directory_index(std::string path)
{
	DIR*			dir;
	struct dirent*		entry;

	dir = opendir(path.c_str());
	if (!dir)
		return false;
	_body << "<html><head><title>Index</title></head><body><h1>Index of " << _request->_uri << "</h1><ul>";

	std::string href;
	href.reserve(256);
	std::string link;
	href.reserve(256);

	while ((entry = readdir(dir)) != NULL)
	{
		href = entry->d_name;
		if (href == "." || href == "..")
			continue;

		link = href;
		int eflags = Io::file_stat(path + href);

		if (eflags & FS_ISDIR) {
			href += "/";
			link = "/" + href;
		}
		_body << "<li><a href=\"" << href << "\">" << link << "</a></li>";
	}
	_body << "</ul></body></html>";

	closedir(dir);
	return true;
}

std::string Response::get_content_type(std::string uri)
{
	size_t pos = uri.find_last_of(".");
	if (pos != std::string::npos)
	{
		std::string extension = uri.substr(pos);
		if (mime_map.count(extension) > 0)
			return mime_map[extension];
	}
	return mime_map[".html"];
}
