#include "HttpRequest.hpp"

ParseResult HttpRequest::parse_trailer_header( std::string &body )
{
    S_Header head;
    while (!body.empty() && find_and_get(body, head.key, ":")) { // handle the trailer_headers
        if (!check_valid_spaces(body, 1))
        {
            throw (BadRequest);
        }
        
        if (!find_and_get(body, head.value, "\r\n"))
        {
            throw (BadRequest);
        }
        
        if (head.key.empty() || header_invalid_chars(head.key, head.value))
        {
            throw (BadRequest);
        }
        r_header.push_back(head);
    }
    return ( OK );
}

ParseResult HttpRequest::parse_chunked_body( std::string &tmp )
{
    std::string body = r_body;

    while ( r_read_status != END )
    {
        std::string chunk_size;
        std::string chunk_data = "";

        if (!find_and_get( body, chunk_size, "\r\n" ) )
            throw ( Incomplete );
        
        int size = hex_to_int(chunk_size); 

        if (size < 0)
            throw (BadRequest);
        
        if (size == 0)
        {
            r_read_status = END;
            break ;
        }
        
        if (static_cast<int>(body.size()) < size + 2)  // chunk + "\r\n"
        {
            throw ( Incomplete );

        }

        if (body[size] != '\r' || body[size + 1] != '\n')
            throw ( BadRequest );

        tmp.append(body, 0, size); 
        body.erase(0, size + 2);
    }
    
    ParseResult res = parse_trailer_header(body);
    find_and_get( body, body, "\r\n" );
    if (!body.empty())
        throw (BadRequest);
    return res;
}

ParseResult HttpRequest::parse_body()
{
    if ( r_method != "POST" )
        return ( OK );
    std::string tmp = "";
    if (!r_body.empty() && !r_has_content_length && !r_has_transfer_encoding) // there is a body but there is not content-length and no transfere-encoding
        return (BadRequest);
    if (r_has_transfer_encoding)
    {   
        this->parse_chunked_body( tmp );
        r_body = tmp;
        return ( OK );
    }
    // else if (r_has_content_length == true && r_body.length() < r_content_length)
    //     throw (BadRequest);
    else if (r_has_content_length == true && r_current_body_size < r_content_length)
    {
        throw (Incomplete);
    }

    if (check_repeated_key(r_header) != OK) // check if there is a duplicate of keys (some keys should not be duplicate)
        throw (BadRequest);
    r_read_status = END;
    // std::cout << r_body << std::endl;
    return (OK);
}