#include "HttpRequest.hpp"

ParseResult find_server_location(HttpRequest *http)
{
    ParseResult result= http->setServer(); // setServer --> match the server and return ParseResult
    if ( result != OK )
        return (result);
    result = http->setLocation();
    if ( result != OK )
        return (result);
    return ( OK ); // setLocation --> find the best location based on request URI, and return ParseResult
}

ParseResult HttpRequest::parse(std::string request)
{
    std::string start_line, header;

    if (!find_and_get(request, start_line, "\r\n") || !find_and_get(request, header, "\r\n\r\n"))
        return (BadRequest);

    r_body = request;

    if (parse_start_line(start_line) != OK)
        return (BadRequest);

    if (parse_header(header) != OK)
        return (BadRequest);

    if(r_method != "GET" && parse_body() != OK)
        return (BadRequest);

    return ( find_server_location(this) );
 

}