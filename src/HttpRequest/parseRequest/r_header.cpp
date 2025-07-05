#include "HttpRequest.hpp"

/*
    bool    find_and_get(std::string &request, std::string &dest, std::string delimeter)
    find_and_get searches for the specified delimiter in the request string.
    If the delimiter is found, the function copies the substring from the start
    of 'request' up to (but not including) the delimiter into 'dest'.
    removes the substring including the delimiter from 'request'.
    Returns true if the delimiter exists in 'request'; otherwise, returns false.
*/

ParseResult HttpRequest::parse_header(std::string &header)
{
    header+= "\r\n";
    while (!header.empty()) {
        S_Header head;
        if (!check_valid_spaces(header, 0))
            return (BadRequest);

        if (!find_and_get(header, head.key, ":"))
            return (BadRequest);

        if (!check_valid_spaces(header, 1))
            return (BadRequest);

        if (!find_and_get(header, head.value, "\r\n"))
            return (BadRequest);

        if (head.key.empty() || header_invalid_chars(head.key, head.value))
            return (BadRequest);

        if (to_lower(head.key) == "content-length")
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

        if (to_lower(head.key) == "transfer-encoding")
        {
            if (head.value.empty())
                return (BadRequest);
            r_has_transfer_encoding = true;
        }
        r_header.push_back(head);
    }
    return (OK);
}