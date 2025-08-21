#include "HttpRequest.hpp"

/*
    bool    find_and_get(std::string &request, std::string &dest, std::string delimeter)
    find_and_get searches for the specified delimiter in the request string.
    If the delimiter is found, the function copies the substring from the start
    of 'request' up to (but not including) the delimiter into 'dest'.
    removes the substring including the delimiter from 'request'.
    Returns true if the delimiter exists in 'request'; otherwise, returns false.
*/

static int from_hex(char ch) {
    if (ch >= '0' && ch <= '9') return ch - '0';
    if (ch >= 'A' && ch <= 'F') return ch - 'A' + 10;
    if (ch >= 'a' && ch <= 'f') return ch - 'a' + 10;
    return -1;
}

static std::string url_decode(const std::string &src, bool decode_plus = false) {
    std::string result;
    result.reserve(src.size());

    for (std::string::size_type i = 0; i < src.size(); ++i) {
        if (src[i] == '%' && i + 2 < src.size()) {
            int hi = from_hex(src[i + 1]);
            int lo = from_hex(src[i + 2]);
            if (hi != -1 && lo != -1) {
                result += static_cast<char>((hi << 4) | lo);
                i += 2;
            } else {
                result += src[i];
            }
        } else if (decode_plus && src[i] == '+') {
            result += ' ';
        } else {
            result += src[i];
        }
    }

    return result;
}



bool is_valid_uri(const std::string& URI) {
    for (size_t i = 0; i < URI.size(); i++) {
        unsigned char c = static_cast<unsigned char>(URI[i]);
        if ( is_invalid_key_char(c) ||  c == ' ')
            return false;
    }
    return true;
}


ParseResult contains_path_traversal( std::string &uri )
{
    if ( uri.find("..") != std::string::npos )
        throw ( Forbidden ) ;
    size_t index = uri.find("/./");
    if ( index != std::string::npos )
        uri.erase(index, 2);
    return ( OK );
}

ParseResult validate_path(std::string &path)
{
    size_t len = path.length() - 1;

    if ( path.empty() || path[0] != '/' )
        throw (BadRequest);

    // if ( path[len] != '/' )
    //     path += "/";

    for (size_t i = 0; i < len; i++)
    {
        if ( static_cast<unsigned char>(path[i]) != '\0' && (is_invalid_key_char(static_cast<unsigned char>(path[i])) || static_cast<unsigned char>(path[i]) == ' '))
            throw ( BadRequest );

        if ( i < len && path[i] == '/' && path[i + 1] && path[i + 1] == '/' )
        {
            path.erase(i + 1, 1);
            i--;
        }
    }
    return ( contains_path_traversal ( path ) );
}

ParseResult HttpRequest::parse_start_line(std::string &start_line)
{
    std::string method, url, version;

    if (!find_and_get(start_line, method, " "))
        throw (BadRequest);

    if (!find_and_get(start_line, url, " ") || !is_valid_uri(url))
    {
        throw (BadRequest);
    }

    version = start_line;
    if (version.find(" ") != std::string::npos)
        throw (BadRequest);
    
    if (version != "HTTP/1.0" && version != "HTTP/1.1")
        throw (HTTPVersionNotSupported);

    if (method != "GET" && method != "POST" && method != "DELETE")
        throw (NotImplemented);

    size_t index = url.find("#");
    if ( index != std::string::npos )
        url.erase( index );

    r_method = method; r_url = url; r_version = version;


    if (find_and_get(url, r_url, "?")){
        r_query = url;
    }
    r_url = url_decode(r_url);
    r_query = url_decode(r_query, true); 
    return ( validate_path(r_url) );
}