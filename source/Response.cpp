#include <Response.hpp>
#include <Defines.hpp>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <ctime>

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

Response::Response(Request *req)
{
	_http_code = STATUS_NOT_FOUND;
	if (req->_error)
	{
		build_response(req, req->_error);
		return;
	}

	switch (req->_method)
	{
		case METHOD_GET: handle_get(req); break;
		case METHOD_POST: handle_post(req); break;
		case METHOD_DELETE: handle_delete(req); break;
	}

	if (_http_code == STATUS_NOT_FOUND)
	{
		read_www_file("./www/404.html");
	}
	std::cout << _http_code << std::endl;
	build_response(req, _http_code);
}

void	Response::handle_post(Request *req)
{
	(void)req;
	std::cout << "handle_post()\n";
}

void	Response::handle_delete(Request *req)
{
	(void)req;
	std::cout << "handle_delete()\n";
}

void	Response::handle_get(Request *req)
{
	struct stat sb;

	std::cout << "handle_get()\n";
	std::string filename = "./www" + req->_uri;

	if(stat(filename.c_str(), &sb) == 0)
	{
		if (S_ISREG(sb.st_mode))
		{
			_http_code = STATUS_OK;
			read_www_file(filename);
		}
		else if (S_ISDIR(sb.st_mode))
		{
			_http_code = STATUS_OK;
			directory_index(req, filename);
		}
	}
}

bool	Response::read_www_file(std::string filename)
{
	std::cout << "read_www_file: " << filename << "\n";
	std::ifstream file(filename, std::ios::binary);
	if (!file)
		return false;
	_body << file.rdbuf();
	file.close();
	std::cout << "_body size: " << _body.str().size();
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

std::string Response::date_now(void)
{  
	time_t t;
	struct tm *time_struct;
	char buf[128];

	std::time(&t);
	time_struct = std::gmtime(&t);
	std::strftime(buf, sizeof(buf) - 1, "%a, %d %b %Y %H:%M:%S GMT", time_struct);
	return std::string(buf);
}

void	Response::build_response(Request *req, int status)
{
	const std::string &bs = _body.str();

	buffer << "HTTP/1.1 " << status << " " << code_map[status] << CRLF;
	buffer << "Content-Length: " << bs.size() << CRLF;
	buffer << "Connection: close" << CRLF;
	buffer << "Date: " << date_now() << CRLF;
	buffer << "Server: webserv" << CRLF;

	if (bs.size() > 0)
		buffer << "Content-Type: " << get_content_type(req->_uri) << CRLF;
	buffer << CRLF;
	buffer << bs;
}

void	Response::directory_index(Request *req, std::string path)
{
	DIR*			dir;
	struct dirent*		entry;

	dir = opendir(path.c_str());
	if (!dir)
		return;
	_body << "<html><head><title>Index</title></head><body><h1>Index of " << req->_uri << "</h1><ul>";

	std::string e;
	e.reserve(256);
	while ((entry = readdir(dir)) != NULL)
	{
		e = entry->d_name;
		if (e == "." || e == "..")
			continue;
		_body << "<li><a href=\"" << e << "\">" << e << "</a></li>";
	}
	_body << "</ul></body></html>";

	closedir(dir);
}
