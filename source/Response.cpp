#include <Response.hpp>
#include <Defines.hpp>
#include <dirent.h>

std::unordered_map<int, std::string> code_map =
		{{ 100, "Continue"},
		{ 101, "Switching Protocols"},
		{ 102, "Processing"},

		{ 200, "OK"},
		{ 201, "Created"},
		{ 202, "Accepted"},
		{ 203, "Non-authoritative Information"},
		{ 204, "No Content"},
		{ 205, "Reset Content"},
		{ 206, "Partial Content"},
		{ 207, "Multi-Status"},
		{ 208, "Already Reported"},
		{ 226, "IM Used"},

		{ 300, "Multiple Choices"},
		{ 301, "Moved Permanently"},
		{ 302, "Found"},
		{ 303, "See Other"},
		{ 304, "Not Modified"},
		{ 305, "Use Proxy"},
		{ 307, "Temporary Redirect"},
		{ 308, "Permanent Redirect"},

		{ 400, "Bad Request"},
		{ 401, "Unauthorized"},
		{ 402, "Payment Required"},
		{ 403, "Forbidden"},
		{ 404, "Not Found"},
		{ 405, "Method Not Allowed"},
		{ 406, "Not Acceptable"},
		{ 407, "Proxy Authentication Required"},
		{ 408, "Request Timeout"},
		{ 409, "Conflict"},
		{ 410, "Gone"},
		{ 411, "Length Required"},
		{ 412, "Precondition Failed"},
		{ 413, "Payload Too Large"},
		{ 414, "Request-URI Too Long"},
		{ 415, "Unsupported Media Type"},
		{ 416, "Requested Range Not Satisfiable"},
		{ 417, "Expectation Failed"},
		{ 418, "I’m a teapot"},
		{ 421, "Misdirected Request"},
		{ 422, "Unprocessable Entity"},
		{ 423, "Locked"},
		{ 424, "Failed Dependency"},
		{ 426, "Upgrade Required"},
		{ 428, "Precondition Required"},
		{ 429, "Too Many Requests"},
		{ 431, "Request Header Fields Too Large"},
		{ 444, "Connection Closed Without Response"},
		{ 451, "Unavailable For Legal Reasons"},
		{ 499, "Client Closed Request"},
		
		{ 500, "Internal Server Error"},
		{ 501, "Not Implemented"},
		{ 502, "Bad Gateway"},
		{ 503, "Service Unavailable"},
		{ 504, "Gateway Timeout"},
		{ 505, "HTTP Version Not Supported"},
		{ 506, "Variant Also Negotiates"},
		{ 507, "Insufficient Storage"},
		{ 508, "Loop Detected"},
		{ 510, "Not Extended"},
		{ 511, "Network Authentication Required"},
		{ 599, "Network Connect Timeout Error"}};

std::unordered_map<std::string, std::string> mime_map =
			     {{".html",  "text/html"},
			     {".xml",   "text/xml"},
			     {".xhtml", "application/xhtml+xml"},
			     {".txt",   "text/plain"},
			     {".rtf",   "application/rtf"},
			     {".pdf",   "application/pdf"},
			     {".word",  "application/msword"},
			     {".png",   "image/png"},
			     {".gif",   "image/gif"},
			     {".jpg",   "image/jpeg"},
			     {".jpeg",  "image/jpeg"},
			     {".au", "audio/basic"},
			     {".mpeg", "video/mpeg"},
			     {".mpg", "video/mpeg"},
			     {".avi", "video/x-msvideo"},
			     {".gz", "application/x-gzip"},
			     {".tar", "application/x-tar"},
			     {".svg", "image/svg+xml"},
			     {".css", "text/css"},
			     {"", "text/plain"},
			     {"default", "text/plain"}};

Response::~Response()
{
}

Response::Response(std::shared_ptr<Request> request) : _request(request), _status_code(STATUS_NOT_FOUND)
{
	int error_code = this->has_errors();

	if (error_code) {
		set_error_page(error_code);
		create_response(error_code);
		return;
	}

	if (!_location->_redirectPath.empty()) {
		std::cout << "Location: " << _location->_rootPath << std::endl;
		create_response(_location->_redirectCode);
		return;
	}

	if (Cgi::is_cgi(_location, _request->_uri)) {
		std::cout << "CGI: " << _request->_uri;
		do_cgi();
		create_response(_status_code);
		return;
	}
	switch (_request->_method) {
		case METHOD_GET: handle_get(); break;
		case METHOD_POST: handle_post(); break;
		case METHOD_DELETE: handle_delete(); break;
	}
	set_error_page(_status_code);
	create_response(_status_code);
}

int	Response::has_errors(void)
{
	if (_request->_error) {
		return _request->_error;
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
	buffer << CRLF;
	buffer << bs;
}

void	Response::handle_get(void)
{
	std::string filename = _location->_rootPath + _request->_uri;
	int flags = Io::file_stat(filename);

	if (!(flags & FS_READ))
	{
		_status_code = STATUS_NOT_FOUND;
		return;
	}
	if (flags & FS_ISDIR) {
		if (_request->_uri.back() != '/' || !_location->_autoIndex)
			return;
		if (directory_index(filename))
			_status_code = STATUS_OK;
		return;
	}
	if (flags & FS_ISFILE && Io::read_file(filename, _body))
		_status_code = STATUS_OK;
}

void	Response::handle_post(void)
{
	for (auto & part : _request->parts)
	{
		if (part.data.empty() || part.filename.empty())
			continue;
		if (Io::write_file(_location->_uploadPath + "/" + part.filename, part.data))
			_status_code = STATUS_OK;
	}
}

void	Response::handle_delete(void)
{
	std::cout << "handle_delete()\n";
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

	std::string e;
	e.reserve(256);
	while ((entry = readdir(dir)) != NULL)
	{
		e = entry->d_name;
		if (e == "." || e == "..")
			continue;
		std::string link = _request->_uri;
		if (link.back() != '/')
			link += "/";
		link += e;
		_body << "<li><a href=\"" << link << "\">" << e << "</a></li>";
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


void Response::do_cgi(void)
{	
	Cgi cgi(_location, _request);

	std::string output;
	if (!cgi.execute(output))
	{
		_status_code = 404;
		std::cout << "Cgi.execute failed\n";
		return;
	}
	_body << output;
	_status_code = 200;
	std::cout << "CGI RESPONSE: " << output << std::endl;
}
