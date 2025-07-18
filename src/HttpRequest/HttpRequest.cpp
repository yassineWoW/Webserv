#include "HttpRequest.hpp"


HttpRequest::HttpRequest(): r_buffer(""), r_current_body_size(0), r_method(""), r_url(""), r_version(""),  r_query(""),  r_body(""), r_host(""), r_content_type(""), path(""), r_auto_index(false), r_content_length(0), r_has_content_length(false), r_has_transfer_encoding(false), r_status_code(OK), r_read_status(Start_Line), r_keep_alive(true)  { }


ServerConfig&   HttpRequest::getServer() { return ( this->server ); }

ParseResult HttpRequest::setServer() 
{
    std::vector<ServerConfig> servers = (ConfigParser::getInstance("webserv.conf"))->getServers();
    
    if (servers.size() == 0)
        return ( InternalError );
    
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
        if ( r_url.compare(0, current_path.length(), current_path) == 0 && current_path.length() >= bestPrefix )  // /student   /studen/index.html
        {
            bestPrefix = current_path.length();
            this->location = locations[i];
        }
    }

    if (bestPrefix > 0)
    {
        if (r_url.length() > location.path.length()) // /student   /studen/index.html
        {
            if (r_url[location.path.length() - 1] != '/')
            {
                return ( NotFound );
            }
        }
    }

    std::vector<std::string> allowed_methods = location.allowed_methods;
    if ( allowed_methods.empty() )
        return ( OK );
    bool flag = false;
    for ( size_t i = 0; i < location.allowed_methods.size(); i++ )
    {
        if ( location.allowed_methods[i] == r_method )
        {
            flag = true;
            break;
        }
    }

    if ( !flag )
    {
        return ( NotAllowed );
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

E_STATUS & HttpRequest::getReadStatus( ) { return ( r_read_status ); }
void       HttpRequest::setReadStatus( E_STATUS status ) { r_read_status = status; }

size_t &        HttpRequest::getCurrentBodySize( ) { return (r_current_body_size); } ;
void            HttpRequest::setCurrentBodySize( size_t size ) { r_current_body_size += size; } ;

std::string &   HttpRequest::getBody( ) { return ( r_body ); };

bool &          HttpRequest::getKeepAlive( ) { return ( r_keep_alive ); } ;

std::string &   HttpRequest::getContentType( ) { return ( r_content_type ); };

bool &                       HttpRequest::getHasContentLength( ) { return ( r_has_content_length ); };
bool &                       HttpRequest::getHasTransferEncoding( ) { return ( r_has_transfer_encoding); };
ParseResult &                HttpRequest::getStatusCode( ) { return (  r_status_code); };
std::string &                HttpRequest::getUri( ) { return ( r_url ); };
std::string &                HttpRequest::getMethod( ) { return ( r_method ); };
bool &                       HttpRequest::getAutoIndex( ) { return ( r_auto_index ); };
std::vector<std::string> &   HttpRequest::getAutoIndexFiles( ) { return ( r_auto_index_files ); };
HttpRequest::~HttpRequest () { }