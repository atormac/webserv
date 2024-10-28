#include <Request.hpp>

Request::Request()
{
	//this->state = S_START;
	//this->data = data;
}

Request::~Request()
{
}

enum {
	METHOD,
	DONE,
};

int Request::Parse(std::string data)
{
	int state = METHOD;
	(void)state;
	std::cout << data << std::endl;
	/*
	for (char c in data)
	{
		switch (state)
		{
			case S_START:
		}
	}
	*/
	return (1);
}
