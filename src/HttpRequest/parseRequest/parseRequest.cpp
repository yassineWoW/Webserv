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

    std::cout << "-------------- START LINE END-------------\n";

    if (parse_header(header) != OK)
        return (BadRequest);

    std::cout << "-------------- header LINE END-------------\n";

    if(r_method != "GET" && parse_body() != OK)
        return (BadRequest);

    std::cout << "-------------- BODY LINE END-------------\n";

    ParseResult result = find_server_location(this);
    if ( result != OK )
        return (result);
    
    check_valid_path( );

    std::cout << "\n\n------------------------BODY-----------------------------\n\n" << std::endl;

    std::cout << r_body << std::endl;

    std::cout << "\n\n-------------------------END----------------------------\n\n" << std::endl;

    return ( OK );
}