#include "HttpRequest.hpp"

ParseResult HttpRequest::parse_body()
{
    std::string tmp;

    if (!r_body.empty() && !r_has_content_length && !r_has_transfer_encoding) // there is a body but there is not content-length and no transfere-encoding
        return (BadRequest);

    if (r_has_transfer_encoding)
    {
        if (r_body.find("0\r\n") == std::string::npos){
            std::cout << "BADRUQUEST NUMBER\n" << std::endl;
            return (BadRequest);
        }

        std::string body = r_body;

        while (!body.empty())
        {
            std::string chunk_size;
            std::string chunk_data;
            if (!find_and_get(body, chunk_size, "\r\n")){
                std::cout << "BADRUQUEST NUMBER\n" << std::endl;
                return (BadRequest);
            }
            try {
                int size = hex_to_int(chunk_size); 
                if (size < 0){
                    std::cout << "BADRUQUEST NUMBER\n" << std::endl;
                    return (BadRequest);
                }
                if (size == 0)
                    break;
                if (!find_and_get(body, chunk_data, "\r\n")){
                    std::cout << "BADRUQUEST NUMBER\n" << std::endl;
                    return (BadRequest);
                }
                tmp.append(chunk_data, 0, size);

            }
            catch (std::exception &e){
                return (BadRequest);
            }
           
        }
        
        S_Header head;
        while (!body.empty() && find_and_get(body, head.key, ":")) { // handle the trailer_headers
            if (!check_valid_spaces(body, 1))
                return (BadRequest);
            if (!find_and_get(body, head.value, "\r\n"))
                return (BadRequest);
            if (head.key.empty() || header_invalid_chars(head.key, head.value))
                return (BadRequest);
            r_header.push_back(head);
        }
    }
    else if (r_has_content_length == true && r_body.length() != r_content_length)
        return (BadRequest);

    r_body = tmp;

    if (check_repeated_key(r_header) != OK) // check if there is a duplicate of keys (some keys should not be duplicate)
        return (BadRequest);
    return (OK);
}