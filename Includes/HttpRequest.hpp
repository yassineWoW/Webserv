#ifndef REQUEST_HPP
#define REQUEST_HPP

#include "includes.hpp"

struct S_Header {
    std::string key;
    std::string value;
};

enum ParseResult { OK, BadRequest, NotAllowed,Incomplete };

class HttpRequest
{
    std::string             r_method, r_url, r_version, r_query;
    std::vector<S_Header>   r_header;
    std::string             r_body;
    unsigned int            r_content_length;
    bool                    r_has_content_length;
    bool                    r_has_transfer_encoding;
    public:
        HttpRequest();
        ParseResult parse(std::string request);
        ParseResult parse_start_line(std::string &start_line);
        ParseResult parse_header(std::string &header);
        ParseResult parse_body();
        ~HttpRequest();

};

bool        find_and_get(std::string &request, std::string &dest, std::string delimeter) ;
bool        check_valid_spaces(std::string &to_check, int authorized_space_number) ;
std::string to_lower(std::string header) ;
ParseResult check_repeated_key(std::vector<S_Header>& header) ;
bool        is_invalid_key_char(unsigned char c) ;
bool        is_invalid_value_char(unsigned char c) ;
bool        header_invalid_chars(const std::string& key, const std::string& value) ;
int         hex_char_to_int(char c) ;
int         hex_to_int(const std::string& hex_str) ;

#endif