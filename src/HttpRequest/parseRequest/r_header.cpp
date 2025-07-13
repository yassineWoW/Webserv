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
    std::string required_headers[] = { "host", "user-agent", "accept", (r_method == "POST"? "content-type" : "END"), "END" };
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
            return (false);
    }

    return (true);
}


ParseResult HttpRequest::parse_header(std::string &header)
{
    std::cout << header << std::endl;
    header+= "\r\n";
    while (!header.empty()) {
        S_Header head;
        if (!check_valid_spaces(header, 0))
            return (BadRequest);

        if (!find_and_get(header, head.key, ":"))
            return (BadRequest);

        head.key = to_lower(head.key);

        if (!check_valid_spaces(header, 1))
            return (BadRequest);

        if (!find_and_get(header, head.value, "\r\n"))
            return (BadRequest);

        if (head.key.empty() || header_invalid_chars(head.key, head.value))
        {
            return (Conflict);
        }

        if (head.key == "content-length")
        {
            for (size_t i = 0; i < head.value.length(); i++) {
                if (i == 0 && head.value[i] == ' ')
                    continue;
                if (!std::isdigit(head.value[i]))
                    return (BadRequest);
            }
            long long tmp = atoll(head.value.c_str());
            if (tmp < 0 || tmp > UINT_MAX)
                return (BadRequest);
            r_content_length = tmp;
            r_has_content_length = true;
        }

        if (head.key == "transfer-encoding")
        {
            if (head.value.empty())
                return (BadRequest);
            r_has_transfer_encoding = true;
        }

        if (head.key == "host")
            r_host = head.value;
        if (head.key == "content-type")
            r_content_type = head.value;

        r_header.push_back(head);
    }
    if (!validate_required_headers( ))
        return (BadRequest);
    return (OK);
}

void test()
{
    int i = 0;
    while (i < 100)
    {
        i++;
    }
}