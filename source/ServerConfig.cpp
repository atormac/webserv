#include <ServerConfig.hpp>
#include <Location.hpp>
#include <memory>
#include <signal.h>

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

		if (line != "}" && element != "location" && element != "server_name" && element != "client_max_body_size" && element != "error_page")
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
		else if (element == "}")
			break;
		else
			throw std::runtime_error("parseServer: Unknown element in server block: " + line);	
	}
	if (element != "}")
		throw std::runtime_error("parseSerever: Unterminated server block!");
}

// Setters
void ServerConfig::_addName(std::stringstream &ss)
{
	std::string name;

	ss >> name;
	if (name.empty())
		throw std::runtime_error("_addName: Adding empty server name!");
	if (name.back() != ';')
		throw std::runtime_error("_addName: server_name line not terminated with semicolon!");
	name.pop_back();
	if (std::find(_names.begin(), _names.end(), name) != _names.end())
		throw std::runtime_error("_addName: Adding duplicate server name!");
	_names.push_back(name);
}

void ServerConfig::_addMaxSize(std::stringstream &ss)
{
	std::string size;

	ss >> size;
	if (size.empty())
		throw std::runtime_error("_addMaxSize: Adding empty max size!");
	if (!validLineEnd(size, ss))
		throw std::runtime_error("_addMaxSize: Unexpected characters in max size line!");

	_maxSize = stringToType<size_t>(size);
}

/*
void ServerConfig::_addErrorPage(std::string &page)
{
	if (page.empty())
		throw std::runtime_error("_addErrorPage: Adding empty error page!");
	if (page.back() != ';')
		throw std::runtime_error("_addErrorPage: Error page not terminated with semicolon!");
	page.pop_back();

	size_t delim = page.find(" ");
	if (delim == std::string::npos)	
		throw std::runtime_error("_addErrorPage: Invalid error page!");
	std::string errorString = page.substr(0, delim);
	std::string path = page.substr(delim + 1);
	unsigned int errorNum = stoUI(errorString);
	if (!fileExists(path))
		throw std::runtime_error("_addErrorPage: Invalid error page path!");	
	_errorPages.insert(std::make_pair(errorNum, path));

	// For debugging	
	std::cout << "errno: " << errorNum << std::endl;
	std::cout << "path: " << path << std::endl;
}
*/

void ServerConfig::_addErrorPage(std::stringstream &ss)
{
	std::string num_str;
	std::string page;
	std::string termin;
	int errorNum;

	ss >> num_str;
	errorNum = stringToType<int>(num_str);
	ss >> page;
	if (page.empty())
		throw std::runtime_error("_addErrorPage: Adding empty error page!");
	if (page.back() == ';')
		page.pop_back();
	else if (ss >> termin, termin != ";")
		throw std::runtime_error("_addErrorPage: Error page not terminated with semicolon!");

	if (!fileExists(page))
		throw std::runtime_error("_addErrorPage: Invalid error page path!");	
	_errorPages.insert(std::make_pair(errorNum, page));

	// For debugging	
	std::cout << "errno: " << errorNum << std::endl;
	std::cout << "path: " << page << std::endl;
}

void ServerConfig::_addLocation(std::shared_ptr<Location> location)
{
	_locations.push_back(location);
}

// Getters
std::vector<std::string> &ServerConfig::getNames() {return _names;}
size_t ServerConfig::getMaxSize() {return _maxSize;}
std::vector<std::shared_ptr<Location>> &ServerConfig::getLocations() {return _locations;}
