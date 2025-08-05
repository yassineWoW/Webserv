#include "HttpRequest.hpp"

/*
    bool    find_and_get(std::string &request, std::string &dest, std::string delimeter)
    find_and_get searches for the specified delimiter in the request string.
    If the delimiter is found, the function copies the substring from the start
    of 'request' up to (but not including) the delimiter into 'dest'.
    removes the substring including the delimiter from 'request'.
    Returns true if the delimiter exists in 'request'; otherwise, returns false.
*/

bool HttpRequest::validate_required_headers( )
{
    std::string required_headers[] = { "host", (r_method == "POST"? "content-type" : "END"), "END" };
    bool connectionFlag = false;

    for (int i = 0; i < 5; i++)
    {
        if (required_headers[i] == "END")
            break ;

        bool flag = false;
        for (std::vector<S_Header>::const_iterator begin = r_header.begin(); begin != r_header.end(); begin++)
        {
            if (!connectionFlag && begin->key == "connection")
            {
                if (begin->value == "close")
                    r_keep_alive = false;
                connectionFlag = true ;
            }
            if (required_headers[i] == begin->key && !begin->value.empty())
            {                
                flag = true;
                break ;
            }
        }
        if (!flag)
            throw ( BadRequest );
    }

    return (true);
}


ParseResult HttpRequest::parse_header(std::string &header)
{
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
    return (OK);
}