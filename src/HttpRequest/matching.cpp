#include "HttpRequest.hpp"
#include "Errors.hpp"

ParseResult find_server_location(HttpRequest *http)
{
    ParseResult result= http->setServer();
    if ( result != OK )
        return (result);
    result = http->setLocation();
    if ( result != OK )
        return (result);
    return ( OK );
}

ParseResult    match_server_location ( HttpRequest &request, std::string &response)
{
    ParseResult ServerResult = find_server_location( &request );
    
    if ( ServerResult != OK)
    {
        Errors errors;
        response = errors.handle_error( request.getServer().error_pages, ServerResult ) ;
    }
    return ( ServerResult );
} 