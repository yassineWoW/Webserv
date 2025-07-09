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

class HttpResponse;
class HttpRequest;
enum ParseResult {
    OK,                         // 0 - Success
    Incomplete,                 // 1 - Waiting for more data (not an error)
    // Common 4xx client-side errors
    BadRequest,                 // 2 - 400
    NotAllowed,                 // 3 - 405
    LengthRequired,             // 4 - 411
    PayloadTooLarge,            // 5 - 413
    URITooLong,                 // 6 - 414
    UnsupportedMediaType,       // 7 - 415
    HeaderFieldsTooLarge,       // 8 - 431
    NotFound,                   // 9 - 404
    Forbidden,                  //10 - 403
    Gone,                       //11 - 410
    Conflict,                   //12 - 409
    RequestTimeout,             //13 - 408

    // Server-side errors
    HTTPVersionNotSupported,    //14 - 505
    InternalError               //15 - 500
};

#endif