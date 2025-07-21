#include "HttpRequest.hpp"

ParseResult find_server_location(HttpRequest *http)
{
    ParseResult result= http->setServer(); // setServer --> match the server and return ParseResult
    if ( result != OK )
        return (result);
    result = http->setLocation();
    if ( result != OK )
        return (result);
    // std::cout <<  "--------------------" << http->getServer().server_name << std::endl;
    return ( OK ); // setLocation --> find the best location based on request URI, and return ParseResult
}

ParseResult HttpRequest::parse( std::string buffer )
{
    std::string start_line, header;
    ParseResult result;

    r_buffer += buffer;
    if ( r_read_status == Start_Line )
    {
        if ( !find_and_get(r_buffer, start_line, "\r\n") )
            throw ( Incomplete );
        result = parse_start_line(start_line);
        if ( result != OK )
            throw ( result );
        r_read_status = Header;
    }

    if ( r_read_status == Header )
    {
        if ( !find_and_get(r_buffer, header, "\r\n\r\n") )
            throw ( Incomplete );  
        try
        {
            result = parse_header(header);
        }
        catch(const ParseResult& e)
        {
            throw e;
        }
        
        if ( result != OK )
            throw ( result );
        
        if ( r_method == "GET" )
            r_read_status = END;
        else
            r_read_status = Body;
        
    }

    if ( r_read_status == Body )
    {
        if ( r_body.empty() )
        {
            r_body = r_buffer;
            r_current_body_size += r_buffer.length();

        }
        else
        {
            r_body += buffer;
            r_current_body_size += buffer.length();
        }
        result = parse_body();
        
        if (result != OK )
        throw ( result );
    }
    
    if ( r_read_status == END )
        return ( OK ) ;
    
    return ( OK );
}