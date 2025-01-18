#include <HttpServer.hpp>

Response::~Response()
{
}

Response::Response(std::shared_ptr<Client> client, std::shared_ptr<Request> req)
	: _request(req)
	, _status_code(STATUS_NOT_FOUND)
{
	this->_client = client;
	int error_code = this->has_errors();

	if (error_code)
	{
		create_response(error_code);
		return;
	}
	if (!_location->_redirectPath.empty())
	{
		create_response(_location->_redirectCode);
		return;
	}

	fix_uri();

	if (Cgi::is_cgi(_location, _request->_uri))
	{
		if (!init_cgi(client))
		{
			_status_code = STATUS_INTERNAL_ERROR;
			finish_response();
			return;
		}
		_status_code = STATUS_OK;
		return;
	}

	if (_location->_session) _handleCookies();

	if (_request->_method == METHOD_GET)
		_status_code = handle_get();
	if (_request->_method == METHOD_POST)
		_status_code = handle_post();
	if (_request->_method == METHOD_DELETE)
		_status_code = handle_delete();
	finish_response();
}

void Response::fix_uri(void)
{
	if (_request->_uri.rfind(_location->_path, 0) == 0)
	{
		_request->_uri.erase(0, _location->_path.size());
		if (_request->_uri.front() != '/')
			_request->_uri = "/" + _request->_uri;
	}
	if (_request->_uri.back() != '/')
		return;
	std::string target = _location->_rootPath + _request->_uri;
	int fs = Io::file_stat(target);

	if (!(fs & FS_ISDIR))
		return;

	if (!_location->_index.empty())
	{
		target = _location->_rootPath + _request->_uri + _location->_index;
		int flags = Io::file_stat(target);

		if (flags & FS_ISFILE)
			_request->_uri += _location->_index;
	}
}

void Response::finish_response(void)
{
	create_response(_status_code);
}

int Response::has_errors(void)
{
	if (_request->parser_error)
		return _request->parser_error;

	_location = find_location();
	if (_location == nullptr)
		return STATUS_NOT_FOUND;

	if (!Request::is_method_allowed(_location->_methods, _request->_method_str))
		return STATUS_METHOD_NOT_ALLOWED;

	if (!_location->_redirectPath.empty())
		return 0;
	try {
		std::filesystem::path root = std::filesystem::canonical(_location->_rootPath);
		std::string filename = _location->_rootPath + _request->_uri;
		std::filesystem::path resolved = std::filesystem::canonical(filename);

		std::string uri_normalized = resolved.generic_string();
		std::string root_normalized = root.generic_string();
		if (uri_normalized.rfind(root_normalized, 0) != 0) //path traversal
			return STATUS_NOT_FOUND;
	} catch (const std::exception &e) {
		return 0;
	}

	return 0;
}

void Response::create_response(int status)
{
	set_error_page(status);

	const std::string &bs = _body.str();
	Logger::log_request(this->_client, bs.size(), status);

	buffer << "HTTP/1.1 " << status << " " << code_map[status] << CRLF;
	buffer << "Content-Length: " << bs.size() << CRLF;
	for (auto const &hdr : _additional_headers)
	{
		buffer << hdr.first << ": " << hdr.second << CRLF;
	}
	buffer << "Connection: close" << CRLF;
	buffer << "Date: " << Str::date_str_now() << CRLF;
	buffer << "Server: " << SERVER_NAME << CRLF;
	if (_location && !_location->_redirectPath.empty())
	{
		buffer << "Location: " << _location->_redirectPath << CRLF;
	}

	if (!_additional_headers.count("Content-Type") && bs.size() > 0)
		buffer << "Content-Type: " << get_content_type(_request->_uri) << CRLF;
	if (!_setCookie.empty())
		buffer << "Set-Cookie: " << _setCookie << CRLF;
	buffer << CRLF;
	buffer << bs;
}


int Response::handle_get(void)
{
	std::string filename = _location->_rootPath + _request->_uri;
	int flags = Io::file_stat(filename);

	if (!flags)
		return STATUS_NOT_FOUND;
	if (!(flags & FS_READ))
		return STATUS_FORBIDDEN;

	if (flags & FS_ISFILE)
	{
		if (!Io::read_file(filename, _body))
			return STATUS_INTERNAL_ERROR;
		return STATUS_OK;
	}
	if (flags & FS_ISDIR)
	{
		if (_request->_uri.back() != '/')
			return STATUS_NOT_FOUND;
		if (!_location->_autoIndex)
		{
			return STATUS_FORBIDDEN;
		}
		if (!directory_index(filename))
			return STATUS_INTERNAL_ERROR;
		return STATUS_OK;
	}
	return STATUS_NOT_FOUND;
}

int Response::handle_post(void)
{
	std::string filename = _location->_rootPath + _request->_uri;
	bool wrote = false;

	if (_request->_body_type == BODY_TYPE_CHUNKED &&
			!_location->_uploadPath.empty()
			&& filename.rfind(_location->_uploadPath, 0) == 0)
	{
		if (!Io::write_file(filename, _request->_body))
			return STATUS_INTERNAL_ERROR;
		return STATUS_CREATED;
	}
	int flags = Io::file_stat(filename);

	for (auto &part : _request->parts)
	{
		if (_location->_uploadPath.empty())
			return STATUS_FORBIDDEN;
		std::string path = _location->_uploadPath + "/" + part.filename;

		if (!Io::write_file(path, part.data))
			return STATUS_INTERNAL_ERROR;
		wrote = true;
	}
	if (wrote)
		return STATUS_CREATED;
	if (!flags)
		return STATUS_NOT_FOUND;
	if (!(flags & FS_READ))
		return STATUS_FORBIDDEN;
	return STATUS_OK;
}

int Response::handle_delete(void)
{
	std::string filename = _location->_rootPath + _request->_uri;
	int flags = Io::file_stat(filename);
	if (!flags)
		return STATUS_NOT_FOUND;
	if (!(flags & FS_WRITE))
		return STATUS_FORBIDDEN;
	if (!std::filesystem::remove(filename.c_str()))
		return STATUS_INTERNAL_ERROR;
	return STATUS_OK;
}

void Response::finish_cgi(std::shared_ptr<Request> req)
{
	if (req->_headers.count("content-type"))
		_additional_headers["Content-Type"] = req->_headers["content-type"];
	if (req->parser_error)
		_status_code = req->parser_error;
	_body << req->_body;
	create_response(_status_code);
}

bool Response::init_cgi(std::shared_ptr<Client> client)
{
	if (_request->_method == METHOD_DELETE)
	{
		_status_code = STATUS_METHOD_NOT_ALLOWED;
		return false;
	}
	Cgi cgi(_location, _request);

	if (!cgi.start(client))
	{
		_status_code = STATUS_INTERNAL_ERROR;
		return false;
	}
	client->conn_type = CONN_WAIT_CGI;
	return true;
}

void Response::set_error(int code)
{
	this->_status_code = code;
}

void Response::set_error_page(int code)
{
	if (!(code >= 400 && code <= 599))
		return;
	_body.str("");
	_body.clear();
	_additional_headers["Content-Type"] = "text/html";

	if (!_request->conf || _request->conf->_errorPages.count(code) == 0)
	{
		generate_error_page(code);
		return;
	}

	std::string page_path = _request->conf->_errorPages[code];
	if (!Io::read_file(page_path, _body))
	{
		generate_error_page(code);
	}
}

void Response::generate_error_page(int code)
{
	std::string msg = std::to_string(code) + " " + code_map[code];
	_body << "<!DOCTYPE html><html><head><title>";
	_body << msg;
	_body << "</title></head><body><h1>";
	_body << msg;
	_body << "</h1></body></html>";
}

std::shared_ptr<Location> Response::find_location(void)
{
	std::shared_ptr<Location> ret;

	if (_request->conf == nullptr)
		return nullptr;
	for (const auto &loc : _request->conf->getLocations())
	{
		if (_request->_uri == loc->_path)
		{
			ret = loc;
			break;
		}
		if (_request->_uri.rfind(loc->_path, 0) == 0 && loc->_path.back() == '/')
		{
			if (!ret || loc->_path.size() > ret->_path.size())
			{
				ret = loc;
			}
		}
	}
	return ret;
}

bool Response::directory_index(std::string path)
{
	DIR *dir;
	struct dirent *entry;

	dir = opendir(path.c_str());
	if (!dir)
		return false;
	_body << "<html><head><title>Index</title></head><body><h1>Index of "
	      << _request->_uri << "</h1><ul>";

	std::string href;
	href.reserve(256);
	std::string link;
	link.reserve(256);

	while ((entry = readdir(dir)) != NULL)
	{
		href = entry->d_name;
		if (href == "." || href == "..")
			continue;

		link = href;
		int eflags = Io::file_stat(path + href);

		if (eflags & FS_ISDIR)
		{
			href += "/";
			link = "/" + href;
		}
		_body << "<li><a href=\"" << href << "\">" << link << "</a></li>";
	}
	closedir(dir);
	_body << "</ul></body></html>";

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
