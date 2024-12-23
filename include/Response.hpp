#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <HttpServer.hpp>

class Location;

class Response
{
	private:
			std::shared_ptr<Request> _request;
			std::shared_ptr <Location>_location; 
			int	_status_code;
			std::string _setCookie;

			int	has_errors(void);
			void	create_response(int status);

			void	handle_post(void);
			void	handle_delete(void);
			void	handle_get(void);

			std::shared_ptr <Location> find_location(void);
			void set_error_page(int code);

			bool	directory_index(std::string path);
			std::string get_content_type(std::string uri);

			bool is_cgi(std::string uri);
			void do_cgi(void);

			void _handleCookies();
			void _validateCookie();
			void _createCookie();
			void _removeInvalidCookie(std::string cookie_path);

			bool init_cgi(Client *client);
	public:
			int status;
			std::ostringstream _body;
			std::ostringstream buffer;

			Response(Client *client, std::shared_ptr<Request> req);
			void finish_response(void);
			void finish_with_body(std::string body);
			~Response();
};
#endif
