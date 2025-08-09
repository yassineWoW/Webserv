#ifndef HTTPRESPONSE_HPP
#define HTTPRESPONSE_HPP

#include "includes.hpp"
#include "HttpRequest.hpp"
#include "multiplexer.hpp"
#include "Errors.hpp"

class HttpResponse {
    public:
        static std::string create_response(ParseResult code, const std::string &body);
        std::string handle_post(HttpRequest& request, std::vector<std::string>& stored_bodies);
        void        handle_get(HttpRequest& request, std::string &response);
        void        handle_delete(HttpRequest& request, std::string &response);
};

#endif