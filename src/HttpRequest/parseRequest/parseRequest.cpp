#include "HttpRequest.hpp"

ParseResult find_server_location(HttpRequest *http)
{
    ParseResult result= http->setServer(); // setServer --> match the server and return ParseResult
    if ( result != OK )
        return (result);
    result = http->setLocation();
    if ( result != OK )
        return (result);
    std::cout <<  "--------------------" << http->getServer().server_name << std::endl;
    return ( OK ); // setLocation --> find the best location based on request URI, and return ParseResult
}

ParseResult HttpRequest::parse( std::string buffer )
{
    std::string start_line, header;
    ParseResult result;

    r_buffer += buffer;
    // std::cout << "\n\n--------------start---------------\n\n" << r_buffer << "\n\n------------------end-----------\n\n" << std::endl;
    if ( r_read_status == Start_Line )
    {
        // std::cout << "------------Start_Line start--------"<<std::endl;

        if ( !find_and_get(r_buffer, start_line, "\r\n") )
            throw ( Incomplete );
        result = parse_start_line(start_line);
        if ( result != OK )
            throw ( result );
        r_read_status = Header;
        // std::cout << "------------Start_Line end--------"<<std::endl;

    }

    if ( r_read_status == Header )
    {

        // std::cout << "------------Header start--------"<<std::endl;
        if ( !find_and_get(r_buffer, header, "\r\n\r\n") )
            throw ( Incomplete );
        // std::cout << "------------Header after--------"<<std::endl;
        
        // std::cout << "------------Header result[" << header << "]--------"<<std::endl;
        result = parse_header(header);
        if ( result != OK )
            throw ( result );
        if ( r_method == "GET" )
            r_read_status = END;
        else
            r_read_status = Body;
        
    }

    if ( r_read_status == Body )
    {
        // std::cout << "------------body start--------"<<std::endl;
        if ( r_body.empty() )
            r_body = r_buffer;
        else
            r_body += buffer;
        r_current_body_size = r_body.length();
        try {
            result = parse_body();
        }
        catch (const ParseResult &e)
        {
            throw ( e ) ;
        }
        // std::cout << "------------body after--------"<<std::endl;
        if (result != OK )
        {
            throw ( result );
        }
    }

    if ( r_read_status == END )
        return ( OK ) ;

      
    // std::string start_line, header;

    // if (!find_and_get(request, start_line, "\r\n") || !find_and_get(request, header, "\r\n\r\n"))
    //     return (BadRequest);

    // r_body = request;

    // if (parse_start_line(start_line) != OK)
    //     return (BadRequest);

    // std::cout << "-------------- START LINE END-------------\n";

    // if (parse_header(header) != OK)
    //     return (BadRequest);

    // std::cout << "-------------- header LINE END-------------\n";

    // if( r_method != "GET" && parse_body() != OK )
    //     return (BadRequest);

    // std::cout << "-------------- BODY LINE END-------------\n";

    // ParseResult result = find_server_location(this);
    // if ( result != OK )
    //     return (result);

    // std::cout << "-------------- Find Server END-------------\n";

    // check_valid_path( );

    // std::cout << "\n\n------------------------BODY-----------------------------\n\n" << std::endl;

    // std::cout << r_body << std::endl;

    // std::cout << "\n\n-------------------------END----------------------------\n\n" << std::endl;

    return ( OK );
}