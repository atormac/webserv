#include <Response.hpp>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>


Response::~Response()
{
}

Response::Response(Request *req)
{
	_http_status = 404;

	if (req->_method == "GET")
	{
		get_resource(req);
	}

	if (_http_status == STATUS_NOT_FOUND)
	{
		read_www_file("./www/404.html");
	}
	std::cout << _http_status << std::endl;
	build_response(_http_status);
}

void	Response::get_resource(Request *req)
{
	struct stat sb;

	std::string filename = "./www" + req->_uri;
	std::cout << "filename: " << filename << std::endl;

	if(stat(filename.c_str(), &sb) == -1)
	{
		_http_status = STATUS_NOT_FOUND;
		return;
	}
	if (S_ISREG(sb.st_mode))
	{
		_http_status = STATUS_OK;
		read_www_file(filename);
	}
	else if (S_ISDIR(sb.st_mode))
	{
		_http_status = STATUS_OK;
		directory_index(filename);
	}
}

bool	Response::read_www_file(std::string filename)
{

	std::ifstream file(filename, std::ios::binary);
	if (!file)
		return false;
	_body << file.rdbuf();
	file.close();
	return true;
}

std::string Response::status_message(int status)
{
	switch (status)
	{
		case STATUS_OK: return"OK";
		case STATUS_NOT_FOUND: return"Not Found";
	}
	return "FATAL";
}

void	Response::build_response(int status)
{
	buffer << "HTTP/1.1 " << status << " " <<  status_message(status) << "\r\n";	
	buffer << "Content-Length: " << _body.str().size() << "\r\n";
	buffer << "\r\n";
	buffer << _body.str().data();
}

void	Response::directory_index(std::string path)
{
	DIR*			dir;
	struct dirent*		entry;

	dir = opendir(path.c_str());
	if (!dir)
		return;
	_body << "<html><head><title>Directory Index</title></head><body><h1>Index of The Directory</h1><ul>";
	while ((entry = readdir(dir)) != NULL)
	{
		_body << "<li><a href=\"" << entry->d_name << "\">" << entry->d_name << "</a></li>";
	}
	_body << "</ul></body></html>";

	closedir(dir);
}
