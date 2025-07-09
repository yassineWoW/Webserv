#include "HttpRequest.hpp"


HttpRequest::HttpRequest(): r_method(""), r_url(""), r_version(""),  r_query(""),  r_body(""), r_host(""), r_content_type(""), path(""), r_content_length(0), r_has_content_length(false), r_has_transfer_encoding(false) { }


ServerConfig&   HttpRequest::getServer() { return ( this->server ); }

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

LocationConfig& HttpRequest::getLocation() { return ( this->location ); }

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

    if (bestPrefix > 0)
    {
        if (r_url.length() > location.path.length())
        {
            if (r_url[location.path.length()] != '/')
                return (NotFound);
        }
    }
    std::string url = r_url; url.erase(0, bestPrefix);
    setPath(this->location.root, url);
    return ( OK );
}

std::string &   HttpRequest::getPath() { return ( this->path );  }

void     HttpRequest::setPath(std::string &root, std::string &url) 
{ 
    this->path = root + (root[root.length() - 1] == '/' ? "" : "/") + (url[0] == '/' ? url.erase(0, 1) : url);
}

std::string &   HttpRequest::getBody( ) { return (this->r_body); }

HttpRequest::~HttpRequest () { }