#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <HttpServer.hpp>

class Location;

class Response {
    private:
	std::shared_ptr<Request> _client;
	std::shared_ptr<Request> _request;
	std::shared_ptr<Location> _location;

	std::map<std::string, std::string> _additional_headers;

	int _status_code;
	std::string _setCookie;

	int has_errors(void);
	void create_response(int status);

	void set_index(void);

	int handle_post(void);
	int handle_delete(void);
	int handle_get(void);

	std::shared_ptr<Location> find_location(void);
	void set_error_page(int code);
	void generate_error_page(int code);

	bool directory_index(std::string path);
	std::string get_content_type(std::string uri);

	bool is_cgi(std::string uri);
	void do_cgi(void);

	void _handleCookies();
	void _validateCookie();
	void _createCookie();
	void _removeInvalidCookie(std::string cookie_path);

	bool init_cgi(std::shared_ptr<Client> client);

    public:
	std::ostringstream _body;
	std::ostringstream buffer;

	Response(std::shared_ptr<Client> client, std::shared_ptr<Request> req);
	void finish_response(void);
	void finish_cgi(std::shared_ptr<Request> req_cgi);
	void set_error(int code);
	~Response();
};
#endif
