#ifndef SERVER_HPP
#define SERVER_HPP

#include <iostream>
#include <Request.hpp>

class Response
{
	public:
		Response(Request *req);
		~Response();
};
#endif
