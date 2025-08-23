#include "HttpRequest.hpp"

/*
    bool    find_and_get(std::string &request, std::string &dest, std::string delimeter)
    find_and_get searches for the specified delimiter in the request string.
    If the delimiter is found, the function copies the substring from the start
    of 'request' up to (but not including) the delimiter into 'dest'.
    removes the substring including the delimiter from 'request'.
    Returns true if the delimiter exists in 'request'; otherwise, returns false.
*/

std::string trim(const std::string& str) {
    std::size_t start = str.find_first_not_of(" \t\n\r");
    std::size_t end = str.find_last_not_of(" \t\n\r");
    return (start == std::string::npos) ? "" : str.substr(start, end - start + 1);
}

void        HttpRequest::handle_cookies( )
{

    std::vector<S_Header>::const_iterator it;
    for (it = r_header.begin(); it != r_header.end(); it++)
    {
        if (it->key == "cookie")
        {
            break;
        }
    }
    if ( it != r_header.end() )
    {
        std::string cookies = it->value;
        while ( !cookies.empty() )
        {
            trim( cookies );
            std::size_t index = cookies.find("=");
            std::size_t sep = cookies.find(";");
            if ( index == std::string::npos )
                break;
            sep = ( sep != std::string::npos ? sep : cookies.length() );
            std::string key = trim( cookies.substr( 0, index ) );
            std::string value = trim( cookies.substr( index + 1, sep - index - 1 ) );
            if (!key.empty() && !value.empty())
                r_cookies[key] = value;
            cookies.erase(0, sep + 1);
        }
    }
    // std::map<std::string, std::string>::iterator begin = r_cookies.begin();
    // std::map<std::string, std::string>::iterator end = r_cookies.end();
    // std::cout << "------------------------------------" <<std::endl;
    // while (begin != end)
    // {
    //     std::cout << begin->first << "=" << begin->second << std::endl;
    //     begin ++;
    // }
    // std::cout << "------------------------------------" <<std::endl;
}


bool HttpRequest::validate_required_headers( )
{
    bool connectionFlag = false;

    bool flag = false;
    for (std::vector<S_Header>::const_iterator begin = r_header.begin(); begin != r_header.end(); begin++)
    {
        if (!connectionFlag && begin->key == "connection")
        {
            if (begin->value == "close")
                r_keep_alive = false;
            connectionFlag = true ;
        }
        if ( begin->key == "host" && !begin->value.empty() )
        {                
            flag = true;
            break ;
        }
    }
    if (!flag)
        throw ( BadRequest );

    return (true);
}


ParseResult HttpRequest::parse_header(std::string &header)
{
    if (header.length() > 1024)
        throw (PayloadTooLarge);
    header+= "\r\n";
    while (!header.empty()) {
        S_Header head;
        if (!check_valid_spaces(header, 0))
            throw (BadRequest);

        if (!find_and_get(header, head.key, ":"))
            throw (BadRequest);

        head.key = to_lower(head.key);

        if (!check_valid_spaces(header, 1))
            throw (BadRequest);

        if (!find_and_get(header, head.value, "\r\n"))
            throw (BadRequest);

        if (head.key.empty() || header_invalid_chars(head.key, head.value))
        {
            throw ( BadRequest );
        }

        if (head.key == "content-length")
        {
            for (size_t i = 0; i < head.value.length(); i++) {
                if (i == 0 && head.value[i] == ' ')
                    continue;
                if (!std::isdigit(head.value[i]))
                    throw (BadRequest);
            }
            long long tmp = atoll(head.value.c_str());
            if (tmp < 0 || tmp > UINT_MAX)
                throw (BadRequest);
            r_content_length = tmp;
            r_has_content_length = true;
        }

        if (head.key == "transfer-encoding")
        {
            if ( head.value.empty() )
                throw (BadRequest);
            r_has_transfer_encoding = true;
        }

        if (head.key == "host")
        {
            size_t index = head.value.find_last_of(":");
            r_host = head.value.substr(0,index);
        }
        if (head.key == "content-type")
            r_content_type = head.value;

        r_header.push_back(head);
    }
    if (!validate_required_headers( ))
        throw (BadRequest);
    handle_cookies();
    return (OK);
}