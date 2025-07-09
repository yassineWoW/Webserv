#ifndef HTTPRESPONSE_HPP
#define HTTPRESPONSE_HPP

#include "includes.hpp"
#include "HttpRequest.hpp"

class HttpRequest;

class HttpResponse {
    public:
        static std::string create_response(ParseResult code, const std::string &body);
        std::string handle_post(HttpRequest& request, std::vector<std::string>& stored_bodies);
};


#endif