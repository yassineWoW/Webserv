#include "HttpRequest.hpp"

/*
    bool    find_and_get(std::string &request, std::string &dest, std::string delimeter)
    find_and_get searches for the specified delimiter in the request string.
    If the delimiter is found, the function copies the substring from the start
    of 'request' up to (but not including) the delimiter into 'dest'.
    removes the substring including the delimiter from 'request'.
    Returns true if the delimiter exists in 'request'; otherwise, returns false.
*/

ParseResult validate_path(std::string &path)
{
    size_t len = path.length() - 1;

    if ( path.empty() || path[0] != '/' )
        return (BadRequest);

    if ( path[len] != '/' )
        path += "/";

    for (size_t i = 0; i < len; i++)
    {
        if ( static_cast<unsigned char>(path[i]) != '\0' && (is_invalid_key_char(static_cast<unsigned char>(path[i])) || static_cast<unsigned char>(path[i]) == ' '))
            return ( BadRequest );

        if ( i < len && path[i] == '/' && path[i + 1] && path[i + 1] == '/' )
        {
            path.erase(i + 1, 1);
            i--;
        }
    }
    return ( OK );
}

ParseResult HttpRequest::parse_start_line(std::string &start_line)
{
    std::string method, url, version;

    if (!find_and_get(start_line, method, " "))
        return (BadRequest);

    if (!find_and_get(start_line, url, " "))
        return (BadRequest);

    version = start_line;
  
    if (version != "HTTP/1.1")
        return (BadRequest);

    if (method != "GET" && method != "POST" && method != "DELETE")
        return (NotAllowed);

    r_method = method; r_url = url; r_version = version;

    if (find_and_get(url, r_url, "?")){
        r_query = url;
    }

    if (validate_path(r_url) != OK)
        return (BadRequest);
    
    return (OK);
}