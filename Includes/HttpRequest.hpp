#ifndef REQUEST_HPP
#define REQUEST_HPP

#include "includes.hpp"
#include "cfileparser.hpp"

struct S_Header {
    std::string key;
    std::string value;
};

enum ParseResult {
    OK,
    BadRequest,         // 400 - Malformed request syntax or invalid headers
    NotAllowed,         // 405 - Method not allowed
    Incomplete,         // Request is not complete yet (waiting for more data)
    URITooLong,         // 414 - URI too long
    PayloadTooLarge,    // 413 - Content-Length too big
    HTTPVersionNotSupported, // 505 - HTTP version not supported
    UnsupportedMediaType,    // 415 - Unsupported Content-Type
    HeaderFieldsTooLarge,    // 431 - Too large header fields
    InternalError       // 500 - Internal server error during parsing
};

class HttpRequest
{
    std::string             r_method, r_url, r_version, r_query;
    std::vector<S_Header>   r_header;
    std::string             r_body;
    unsigned int            r_content_length;
    bool                    r_has_content_length;
    bool                    r_has_transfer_encoding;
    std::string             r_host;
    ServerConfig            server ;
    public:
        HttpRequest();
        ParseResult parse(std::string request);
        ParseResult parse_start_line(std::string &start_line);
        ParseResult parse_header(std::string &header);
        ParseResult parse_body();
        ~HttpRequest();
        ParseResult getServer();

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