#include <Response.hpp>
#include <sys/stat.h>


Response::~Response()
{
}

Response::Response(Request *req)
{
	_http_status = 404;
	if (req->_method == "GET")
	{
		std::string filename = "./www/" + req->_uri;
		if (read_www_file(filename))
			_http_status = 200;
	}
	build_response(_http_status);
}

bool	Response::read_www_file(std::string filename)
{
	struct stat sb;
	if(stat(filename.c_str(), &sb) ==-1 || S_ISREG(sb.st_mode) == 0)
	{
		return false;
	}
	std::ifstream file(filename);

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
