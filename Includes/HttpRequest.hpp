#ifndef REQUEST_HPP
#define REQUEST_HPP


#include "includes.hpp"
#include "HttpResponse.hpp"
#include "cfileparser.hpp"

struct S_Header {
    std::string key;
    std::string value;
};

enum ParseResult {
    OK = 200,                         // 0 - Success
    Incomplete,                 // 1 - Waiting for more data (not an error)
    // Common 4xx client-side errors
    BadRequest = 400,                 // 2 - 400
    NotAllowed = 405,                 // 3 - 405
    LengthRequired = 411,             // 4 - 411
    PayloadTooLarge = 413,            // 5 - 413
    URITooLong = 414,                 // 6 - 414
    UnsupportedMediaType = 415,       // 7 - 415
    HeaderFieldsTooLarge = 431,       // 8 - 431
    NotFound = 404,                   // 9 - 404
    Forbidden = 403,                  //10 - 403
    Gone = 410,                       //11 - 410
    Conflict = 409,                   //12 - 409
    RequestTimeout = 408,             //13 - 408

    // Server-side errors
    InternalError = 500 ,             //14 - 500
    NotImplemented = 501,             //15 - 501
    HTTPVersionNotSupported = 505    //16 - 505
};

enum E_STATUS { Start_Line, Header, Body, END } ; 

class HttpRequest
{
    std::string             r_buffer;
    size_t                  r_current_body_size;
    std::string             r_method, r_url, r_version, r_query;
    std::vector<S_Header>   r_header;
    std::string             r_body;
    std::string             r_host;
    std::string             r_content_type;
    std::string             path;
    unsigned int            r_content_length;
    bool                    r_has_content_length;
    bool                    r_has_transfer_encoding;
    ParseResult             r_status_code;
    E_STATUS                r_read_status;
    ServerConfig            server ;
    LocationConfig          location;
    public:
        HttpRequest( );
        ParseResult parse( std::string request );
        ParseResult parse_start_line( std::string &start_line );
        ParseResult parse_header( std::string &header );
        ParseResult parse_body( );
        ParseResult parse_chunked_body( std::string &tmp );
        ParseResult parse_trailer_header( std::string &body );
        ~HttpRequest( );

        ParseResult     setServer( );
        ParseResult     setLocation( );
        ServerConfig&   getServer( ) ;
        LocationConfig& getLocation( ) ;
        void            setPath( std::string &root, std::string &url ) ;
        std::string &   getPath( ) ;
        E_STATUS &      getReadStatus( ) ;
        void            setReadStatus( E_STATUS status ) ;
        size_t &        getCurrentBodySize( ) ;
        void            setCurrentBodySize( size_t size ) ;
        std::string &   getBody( ) ;
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