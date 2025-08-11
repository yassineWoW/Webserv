#ifndef INCLUDES_HPP
#define INCLUDES_HPP

#include <iostream>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <vector>
#include <set>
#include <map>
#include <stdexcept>
#include <sys/socket.h> // socket, bind, listen
#include <netinet/in.h> // sockaddr_in
#include <arpa/inet.h> // for htons and htonl
#include <sys/epoll.h>
#include <cstring> // for memset hehe
#include <unistd.h>
#include <fcntl.h>
#include <climits>
#include <sys/stat.h>
#include <dirent.h>
#include <signal.h>
#include <ctime>
#include <algorithm>
#include <iomanip>
#include <sys/wait.h>
#include <cctype>

enum ParseResult {
    OK = 200,                         // 0 - Success
    Incomplete,                 // 1 - Waiting for more data (not an error)
    Redirect = 301,
    // Common 4xx client-side errors
    BadRequest = 400,                 // 2 - 400
    NotAllowed = 405,                 // 3 - 405
    LengthRequired = 411,             // 4 - 411
    PayloadTooLarge = 413,            // 5 - 413
    URITooLong = 414,                 // 6 - 414
    UnsupportedMediaType = 415,       // 7 - 415
    HeaderFieldsTooLarge = 431,       // 8 - 431
    NotFound = 404,                   // 9 - 404
    Forbidden = 403,                  //10 - 403
    Gone = 410,                       //11 - 410
    Conflict = 409,                   //12 - 409
    RequestTimeout = 408,             //13 - 408

    // Server-side errors
    InternalError = 500 ,             //14 - 500
    NotImplemented = 501,             //15 - 501
    HTTPVersionNotSupported = 505    //16 - 505
};

template <typename T>
std::string to_string(T value) {
    std::ostringstream oss;
    oss << value;
    return oss.str();
}

ParseResult     isFileAndAccessible(const std::string &path, int accessFlag);
ParseResult     isDirectoryAndAccessible(const std::string &path);
bool            ends_with(const std::string &str, const std::string &suffix);
std::string     create_res(ParseResult code, std::string body, std::string path);
std::string     handle_redirection(const std::string& code, const std::string& url);
#endif