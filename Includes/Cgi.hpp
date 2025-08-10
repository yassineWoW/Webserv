#ifndef CGI_HPP
#define CGI_HPP

#include "includes.hpp"
#include "HttpRequest.hpp"
#include "multiplexer.hpp"
#include "Errors.hpp"

class CGI_handler {
    private :
        std::string script_path;
        std::string interpreter;
    public:
        std::pair<int, pid_t> execute_cgi_script(const std::string& script_path, const std::string& interpreter, HttpRequest& request, bool is_post, int &stdin_fd_out);
        std::string handle_cgi(HttpRequest &request, bool is_post);
};

#endif