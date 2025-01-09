#include <HttpServer.hpp>

ServerConfig::ServerConfig()
	: _maxSize(0)
{
}

ServerConfig::~ServerConfig()
{
}

void ServerConfig::parseServerConfig(std::ifstream &configFile)
{
	std::string line;

	skipEmptyLines(configFile, line);
	if (!configFile)
		throw std::runtime_error("parseServer: Empty server block!");
	if (!std::regex_match(line, std::regex("\\{\\s*")))
		throw std::runtime_error("parseServer: No '{' opening server block!");

	while (skipEmptyLines(configFile, line), configFile)
	{
		std::smatch match_res;
		std::regex ptrn("^\t(\\w+).*");
		if (!std::regex_match(line, match_res, ptrn))
			break;

		if (match_res[1] == "server_name")
			_addName(line);
		else if (match_res[1] == "client_max_body_size")
			_addMaxSize(line);
		else if (match_res[1] == "error_page")
			_addErrorPage(line);
		else if (match_res[1] == "listen")
			_addListen(line);
		else if (match_res[1] == "location")
		{
			std::shared_ptr<Location> location(new Location(this));
			location->parseLocation(configFile, line);
			_addLocation(location);
		} else
			throw std::runtime_error(
				"parseServer: Unknown element in server block: " + line);
	}
	if (!std::regex_match(line, std::regex("\\}\\s*")))
		throw std::runtime_error("parseServer: No '}' closing server block!");
}

// Setters
void ServerConfig::_addName(std::string &line)
{
	std::regex ptrn("^\\tserver_name\\s+([a-zA-Z0-9_.-]*)\\s*;\\s*$");
	std::smatch match_res;

	if (!std::regex_match(line, match_res, ptrn))
		throw std::runtime_error(
			"_addName: Expected format: \"server_name [name];\"");
	if (std::find(_names.begin(), _names.end(), match_res[1]) != _names.end())
		throw std::runtime_error("_addName: Adding duplicate server name!");
	_names.push_back(match_res[1]);
}

void ServerConfig::_addMaxSize(std::string &line)
{
	std::regex ptrn("^\\tclient_max_body_size\\s+(\\d+)(m|M|k|K)?\\s*;\\s*$");
	std::smatch match_res;

	if (_maxSize)
		throw std::runtime_error(
			"_addMaxSize: Cannot add multiple client_max_body_size elements!");
	if (!std::regex_match(line, match_res, ptrn))
		throw std::runtime_error(
			"_addMaxSize: Expected format: \"client_max_body_size [number][optional: k/K/m/M];\"");
	_maxSize = stringToType<size_t>(match_res[1]);
	if (match_res[2] == "k" || match_res[2] == "K")
		_maxSize *= 1024;
	if (match_res[2] == "m" || match_res[2] == "M")
		_maxSize *= 1048576;
}

void ServerConfig::_addErrorPage(std::string &line)
{
	std::regex ptrn(
		"\\terror_page\\s+([1-5][0-9]{2})\\s+(error_pages\\/([1-5][0-9]{2})\\.html)\\s*;\\s*");
	std::smatch match_res;

	if (!std::regex_match(line, match_res, ptrn))
		throw std::runtime_error(
			"_addErrorPage: Expected format: \"error_page [100-599] error_pages/[100-599].html];\"");
	if (_errorPages.count(stringToType<int>(match_res[1])))
		throw std::runtime_error(
			"_addErrorPage: Cannot add multiple error pages to the same error code!");
	if (!fileExists(match_res[2]))
		throw std::runtime_error("_addErrorPage: Invalid error page path!");
	_errorPages.insert(std::make_pair(stringToType<int>(match_res[1]), match_res[2]));
}

void ServerConfig::_addLocation(std::shared_ptr<Location> location)
{
	_locations.push_back(location);
}

void ServerConfig::_addListen(std::string &line)
{
	std::regex ptrn(
		"^\\tlisten\\s+((?:(?:25[0-5]|(?:2[0-4]|1\\d|[1-9]|)\\d)\\.?\\b){4}):([0-9]|[1-9][0-9]{1,3}|[1-5][0-9]{4}|6[0-4][0-9]{3}|65[0-4][0-9]{2}|655[0-2][0-9]|6553[0-5])\\s*;\\s*$");
	std::smatch match_res;

	if (_ipAddress.length() || _port.length())
		throw std::runtime_error(
			"_addListen: Cannot add multiple listen elements!");
	if (!std::regex_match(line, match_res, ptrn))
		throw std::runtime_error(
			"_addListen: Expected format: \"listen [valid ip]:[valid port];\"");
	_ipAddress = match_res[1];
	_port = match_res[2];
}

// Getters
std::vector<std::string> &ServerConfig::getNames()
{
	return _names;
}
size_t ServerConfig::getMaxSize()
{
	return _maxSize;
}
std::string &ServerConfig::getIpAddress()
{
	return _ipAddress;
}
std::string &ServerConfig::getPort()
{
	return _port;
}
std::vector<std::shared_ptr<Location> > &ServerConfig::getLocations()
{
	return _locations;
}
