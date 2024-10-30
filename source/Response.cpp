#include <Response.hpp>

Response::Response(Request *req)
{
	std::cout << "response\n";
	std::cout << req->_method << std::endl;
	if (req->_method == "GET")
	{

	}
}

Response::~Response()
{
}
