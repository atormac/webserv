#include <Response.hpp>
#include <Defines.hpp>
#include <dirent.h>

std::unordered_map<int, std::string> code_map =
			        {{200, "OK"},
				{201, "Created"},
				{400, "Bad Request"},
				{403, "Forbidden"},
				{404, "Not Found"},
				{405, "Method Not Allowed"}};

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
	if (_request->_error)
	{
		create_response(_request->_error);
		return;
	}
	_location = find_location();

	if (_location && !_location->_redirectPath.empty())
	{
		std::cout << "Location: " << _location->_rootPath << std::endl;
		create_response(_location->_redirectCode);
		return;
	}
	if (Cgi::is_cgi(_location, _request->_uri))
	{
		std::cout << "CGI: " << _request->_uri;
		do_cgi();
		create_response(_status_code);
		return;
	}
	switch (_request->_method)
	{
		case METHOD_GET: handle_get(); break;
		case METHOD_POST: handle_post(); break;
		case METHOD_DELETE: handle_delete(); break;
	}
	set_error_page();
	create_response(_status_code);
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
	if (_request->_uri == "/submit")
	{
		for (auto & part : _request->parts)
		{
			if (part.data.empty() || part.filename.empty())
				continue;
			if (Io::write_file(_location->_uploadPath + "/" + part.filename, part.data))
				_status_code = STATUS_OK;
		}
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

void Response::set_error_page(void)
{
	if (!_request->conf)
		return;
	if (_request->conf->_errorPages.count(_status_code) == 0)
		return;
	if (!Io::read_file(_request->conf->_errorPages[_status_code], _body))
		_status_code = 500;
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
