#include "HttpRequest.hpp"


HttpRequest::HttpRequest():r_content_length(0), r_has_content_length(false), r_has_transfer_encoding(false) { }

ParseResult HttpRequest::getServer() 
{
    std::vector<ServerConfig> servers = (ConfigParser::getInstance("webserv.conf"))->getServers();

    size_t i = 0;
    if (servers.size() == 0)
        return (InternalError);

    for (i; i < servers.size(); i++)
    {
        if (r_host == servers[i].server_name )
        {
            server = servers[i];
            break ;
        }
    }

    if (i == servers.size())
        server = servers[0];
    return ( OK );
}

HttpRequest::~HttpRequest () { }