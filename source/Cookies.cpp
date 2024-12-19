#include<HttpServer.hpp>

void file_to_map(std::map<std::string, std::string> &map, std::string path)
{
    std::ifstream in_file(path);    
    std::string key, value;
    std::string line;

    if (!in_file.is_open())
        return;

    while (getline(in_file, line))
    {
        std::stringstream line_stream(line);
        std::getline(line_stream, key, ' ');
        std::getline(line_stream, value);
        map.insert(std::make_pair(key, value));
    }
    in_file.close();
}

void map_to_file(std::map<std::string, std::string> &map, std::string path)
{
    std::ofstream out_file(path);

    if (!out_file.is_open())
        return;

    for (auto &[k, v]: map)
        out_file << k << " " << v << "\n";
    out_file.close();
}

void Response::_validateCookie()
{
    if (_request->_headers.count("cookie") == 0)
        return;

    std::string cookie_path = _request->_headers["cookie"];
    size_t equals = cookie_path.find("=") + 1;
    cookie_path = "./sessions_dir/" + cookie_path.substr(equals);

    int fileStat = Io::file_stat(cookie_path);
    if (!(fileStat & FS_ISFILE) || !(fileStat & FS_READ) || !(fileStat & FS_WRITE))
    {
        _request->_headers.erase(_request->_headers.find("cookie"));
        std::cout << "Removing invalid cookie from request header!\n";
        std::remove(cookie_path.c_str());
        return;
    }

    std::map<std::string, std::string> cookie_map;
    struct tm expire_struct {}, *now_struct;
    time_t now;

    file_to_map(cookie_map, cookie_path);
    
    if (cookie_map.count("expires") != 0)
    {
        strptime(cookie_map["expires"].c_str(), "%a, %d %b %Y %H:%M:%S GMT", &expire_struct);

        std::time(&now);
        now_struct = std::gmtime(&now);

        time_t expire_time = mktime(&expire_struct);
        time_t now_time = mktime(now_struct);
        float diffSecs = difftime(expire_time, now_time);

        //std::cout << "GMT expire time:\t" << asctime(&expire_struct);
        //std::cout << "GMT now time:\t\t" << asctime(now_struct);
        //std::cout << "diff: " << diffSecs << "\n";

        if (0 < diffSecs && diffSecs <= 3600)
        {
            cookie_map["expires"] = Str::date_str_hour_from_now();
            map_to_file(cookie_map, cookie_path);
            return;
        }
    }
    
    _request->_headers.erase(_request->_headers.find("cookie"));
    std::cout << "Removing invalid cookie from request header!\n";
    std::remove(cookie_path.c_str());
}

void Response::_createCookie()
{
    if (_request->_headers.count("cookie") != 0)
        return;

    std::string charSet = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    std::default_random_engine generator(std::random_device{}());
    std::uniform_int_distribution<int> distribution(0, 61);
    std::stringstream session, sessionID;
    
    session << "sessionid=";
    for (int i = 0; i < 64; ++i)
        sessionID << charSet[distribution(generator)];

    std::ofstream sessionFile("./sessions_dir/" + sessionID.str());
    if (!sessionFile.is_open())
    {
        std::cout << "_createCookie: Failed to create session file, aborting cookie setting!\n";
        return;
    }    
    sessionFile << "expires " << Str::date_str_hour_from_now() << "\n";
    sessionFile.close();

    session << sessionID.str() << "; expires=" << Str::date_str_hour_from_now() << "; path=/" << "; host=" << _request->_headers["host"];
    _setCookie = session.str();

    //std::cout << "Setting new session cookie: " << _setCookie << "\n";
}

void Response::_handleCookies()
{
    _validateCookie();
    _createCookie();
}