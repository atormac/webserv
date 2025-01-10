#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <HttpServer.hpp>

class Request;
class Cgi;

enum {
	CONN_REGULAR,
	CONN_WAIT_CGI,
	CONN_CGI,
};

class HttpServer;

class Client {
    private:
	HttpServer *instance;
	time_t _last_active;

    public:
	int conn_type;
	int socket;


	int fd;
	int cgi_read_fd;
	int cgi_write_fd;
	int pid;

	std::string ip_addr;

	std::shared_ptr<Request> req;
	std::shared_ptr<Response> resp;

	std::string response;

	Client();
	~Client();
	Client(HttpServer &inst, int client_fd, int socket_fd, std::string ip);
	Client(HttpServer &inst, int cgi_fd);

	void update_time(void);
	bool has_timed_out(time_t now);
	void cleanup_child(void);
};
#endif
