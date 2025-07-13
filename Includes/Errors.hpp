#ifndef ERRORS_HPP
#define ERRORS_HPP

#include "includes.hpp"

class Errors
{
    public:
        Errors (  ) { };
        std::string handle_error (std::map<int, std::string> error_pages, ParseResult status ) ;
        ~Errors( ) { } ;
};

#endif