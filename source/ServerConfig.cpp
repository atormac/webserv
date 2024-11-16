#include <ServerConfig.hpp>
#include <Location.hpp>
#include <memory>
#include <signal.h>
#include <regex>

int signo = 0;

void signal_handler(int code)
{
	if (code == SIGINT)
		signo = code;
}

ServerConfig::ServerConfig()
{
	signal(SIGINT, ::signal_handler);
}

ServerConfig::~ServerConfig()
{
	std::cout << "ServerConfig deconstructor called" << std::endl;
}

void ServerConfig::parseServerConfig(std::ifstream &configFile)
{
	std::string line;
	std::string element;

	skipEmptyLines(configFile, line);
	if (!configFile)
		throw std::runtime_error("parseServer: Empty server block!");
	if (line != "{")
		throw std::runtime_error("parseServer: No '{' opening server block!");

	while (skipEmptyLines(configFile, line), configFile)
	{
		std::stringstream ss(line);
		ss >> element;

		if (line != "}" && element != "location"
						&& element != "server_name"
						&& element != "client_max_body_size"
						&& element != "error_page"
						&& element != "listen")
			std::cout << "In serv block: |" << element << "|" << std::endl;
		else if (element == "server_name")
			_addName(ss);
		else if (element == "client_max_body_size")
			_addMaxSize(ss);
		else if (element == "error_page")
			_addErrorPage(ss);
		else if (element == "location")
		{
			std::shared_ptr<Location> location(new Location(this));
			location->parseLocation(configFile);
			_addLocation(location);
		}
		else if (element == "listen")
			_addListen(ss);
		else if (element == "}")
			break;
		else
			throw std::runtime_error("parseServer: Unknown element in server block: " + line);	
	}
	if (line != "}")
		throw std::runtime_error("parseSerever: Unterminated server block!");
}

// Setters
void ServerConfig::_addName(std::stringstream &ss)
{
	std::regex ptrn("^\\s*server_name\\s+([a-zA-Z0-9_.-]*)\\s*;\\s*$");
	std::smatch match_res;
	std::string string = ss.str();

	if (!std::regex_match(string, match_res, ptrn))
		throw std::runtime_error("_addName: Expected format: \"server_name [name];\"");
	if (std::find(_names.begin(), _names.end(), match_res.str(1)) != _names.end())
		throw std::runtime_error("_addName: Adding duplicate server name!");
	_names.push_back(match_res.str(1));
}

void ServerConfig::_addMaxSize(std::stringstream &ss)
{
	std::regex ptrn("^\\s*client_max_body_size\\s+(\\d+)\\s*;\\s*$");
	std::smatch match_res;
	std::string string = ss.str();

	if (!std::regex_match(string, match_res, ptrn))
		throw std::runtime_error("_addMaxSize: Expected format: \"client_max_body_size [number];\"");
	_maxSize = stringToType<size_t>(match_res.str(1));
}

void ServerConfig::_addErrorPage(std::stringstream &ss)
{
	std::regex ptrn("\\s*error_page\\s+([1-5][0-9]{2})\\s+(error_pages\\/([1-5][0-9]{2})\\.html)\\s*;\\s*");
	std::smatch match_res;
	std::string string = ss.str();

	if (!std::regex_match(string, match_res, ptrn))
		throw std::runtime_error("_addErrorPage: Expected format: \"error_page [100-599] error_pages/[100-599].html];\"");
	if (!fileExists(match_res.str(2)))
		throw std::runtime_error("_addErrorPage: Invalid error page path!");	
	_errorPages.insert(std::make_pair(stringToType<int>(match_res.str(1)), match_res.str(2)));
	// For debugging	
	std::cout << "errno: " << match_res.str(1) << std::endl;
	std::cout << "path: " << match_res.str(2) << std::endl;
	//
}

void ServerConfig::_addLocation(std::shared_ptr<Location> location)
{
	_locations.push_back(location);
}


void ServerConfig::_addListen(std::stringstream &ss)
{
	std::regex ptrn("^\\s*listen\\s+((?:(?:25[0-5]|(?:2[0-4]|1\\d|[1-9]|)\\d)\\.?\\b){4}):([0-9]|[1-9][0-9]{1,3}|[1-5][0-9]{4}|6[0-4][0-9]{3}|65[0-4][0-9]{2}|655[0-2][0-9]|6553[0-5])\\s*;\\s*$");
	std::smatch match_res;
	std::string string = ss.str();

	if (!std::regex_match(string, match_res, ptrn))
		throw std::runtime_error("_addListen: Expected format: \"listen [valid ip]:[valid port];\"");
	
	_ipAddress = match_res.str(1);
	_port = match_res.str(2);
	// For debugging
	std::cout << _ipAddress << std::endl;	
	std::cout << _port << std::endl;
	//
}

// Getters
std::vector<std::string> &ServerConfig::getNames() {return _names;}
size_t ServerConfig::getMaxSize() {return _maxSize;}
std::vector<std::shared_ptr<Location>> &ServerConfig::getLocations() {return _locations;}
