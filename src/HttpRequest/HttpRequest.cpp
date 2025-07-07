#include "HttpRequest.hpp"


HttpRequest::HttpRequest():r_content_length(0), r_has_content_length(false), r_has_transfer_encoding(false), r_method(""), r_body(""), r_url(""), r_version(""), r_query(""), r_host(""), path("") { }

ParseResult HttpRequest::setServer() 
{
    std::vector<ServerConfig> servers = (ConfigParser::getInstance("webserv.conf"))->getServers();
    
    if (servers.size() == 0)
    return (InternalError);
    
    size_t i;

    for (i = 0; i < servers.size(); i++)
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

ParseResult HttpRequest::setLocation() 
{
    ServerConfig& server = this->getServer();
    std::vector<LocationConfig> locations = server.locations;
    size_t i, bestPrefix = 0;

    for (i = 0; i < locations.size(); i++)
    {
        std::string current_path = locations[i].path;
        if ( r_url.compare(0, current_path.length(), current_path) == 0 && current_path.length() >= bestPrefix )
        {
            bestPrefix = current_path.length();
            this->location = locations[i];
        }
    }
    
    return ( OK );
}

ServerConfig&   HttpRequest::getServer() { return ( this->server ); }

LocationConfig& HttpRequest::getLocation() { return ( this->location ); }

HttpRequest::~HttpRequest () { }