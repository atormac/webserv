#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <HttpServer.hpp>

class Client;

class Logger {
	private:
		static void log_header(void);
	public:
		static void log_request(const std::shared_ptr <Client> &cl, size_t body_size, int status);
};

#endif
