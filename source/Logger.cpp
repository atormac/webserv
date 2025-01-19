#include <Logger.hpp>

void Logger::log_header(void)
{
	auto timestamp = std::chrono::system_clock::now();

	std::time_t now_tt = std::chrono::system_clock::to_time_t(timestamp);
	std::cout << "[webserv] " << std::put_time(std::localtime(&now_tt), "%Y/%m/%d %T ");
}
void Logger::log_request(std::weak_ptr <Request> req, size_t body_size, int status)
{
	auto r = req.lock();
	if (!r)
		return;
	log_header();
	std::cout << "| " << r->_method_str << " " << r->_uri;
	std::cout << " " << r->_body.size() << " " << body_size;
	std::cout << " -> " << status << "\n";
}
