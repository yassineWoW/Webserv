#ifndef REQUEST_HPP
#define REQUEST_HPP


#include "includes.hpp"
#include "HttpResponse.hpp"
#include "cfileparser.hpp"

struct S_Header {
    std::string key;
    std::string value;
};


class HttpRequest
{
    std::string             r_method, r_url, r_version, r_query;
    std::vector<S_Header>   r_header;
    std::string             r_body;
    std::string             r_host;
    std::string             r_content_type;
    std::string             path;
    unsigned int            r_content_length;
    bool                    r_has_content_length;
    bool                    r_has_transfer_encoding;
    ServerConfig            server ;
    LocationConfig          location;
    public:
        HttpRequest( );
        ParseResult parse( std::string request );
        ParseResult parse_start_line( std::string &start_line );
        ParseResult parse_header( std::string &header );
        ParseResult parse_body( );
        ~HttpRequest( );

        ParseResult     setServer( );
        ParseResult     setLocation( );
        ServerConfig&   getServer( ) ;
        LocationConfig& getLocation( ) ;
        void            setPath( std::string &root, std::string &url ) ;
        std::string &   getPath( ) ;
        ParseResult     check_valid_path( ) ;
        std::string &   getBody( ) ;

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