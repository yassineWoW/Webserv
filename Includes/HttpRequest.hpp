#ifndef REQUEST_HPP
#define REQUEST_HPP


#include "includes.hpp"
#include "cfileparser.hpp"

struct S_Header {
    std::string key;
    std::string value;
};

enum E_STATUS { Start_Line, Header, Body, END } ; 

class HttpRequest
{
    std::string                        r_buffer;
    size_t                             r_current_body_size;
    std::string                        r_method, r_url, r_version, r_query;
    std::vector<S_Header>              r_header;
    std::string                        r_body;
    std::string                        r_host;
    std::string                        r_content_type;
    std::string                        path;
    std::vector<std::string>           r_auto_index_files;
    bool                               r_auto_index;
    unsigned int                       r_content_length;
    bool                               r_has_content_length;
    bool                               r_has_transfer_encoding;
    ParseResult                        r_status_code;
    E_STATUS                           r_read_status;
    bool                               r_keep_alive;
    ServerConfig                       server ;
    LocationConfig                     location;
    std::map<std::string, std::string> r_cookies;
    public:
        HttpRequest( );
        ParseResult parse( std::string request );
        ParseResult parse_start_line( std::string &start_line );
        ParseResult parse_header( std::string &header );
        ParseResult parse_body( );
        ParseResult parse_chunked_body( std::string &tmp );
        ParseResult parse_trailer_header( std::string &body );
        bool        validate_required_headers( ) ;
        ParseResult check_valid_path( ) ;
        ParseResult generateAutoindexHtml ( ) ;
        void        handle_cookies();
        ~HttpRequest( );

        // Getters & Setters

        unsigned int getContentLength() const;
        const std::vector<S_Header>& getHeaders() const;
    
        ParseResult                            setServer( );
        ParseResult                            setLocation( );
        ServerConfig&                          getServer( ) ;
        LocationConfig&                        getLocation( ) ;
        void                                   setPath( std::string &root, std::string &url ) ;
        std::string &                          getPath( ) ;
        E_STATUS &                             getReadStatus( ) ;
        void                                   setReadStatus( E_STATUS status ) ;
        size_t &                               getCurrentBodySize( ) ;
        void                                   setCurrentBodySize( size_t size ) ;
        std::string &                          getBody( ) ;
        bool &                                 getKeepAlive( ) ;
        std::string &                          getContentType( ) ;
        bool &                                 getHasContentLength( ) ;
        bool &                                 getHasTransferEncoding( ) ;
        ParseResult &                          getStatusCode( ) ;
        std::string &                          getMethod( );
        std::string &                          getUri( );
        std::string &                          getQuery( );
        bool &                                 getAutoIndex( );
        std::vector<std::string> &             getAutoIndexFiles( );
        std::map<std::string, std::string> &   getCookies( );

        

};

bool            find_and_get(std::string &request, std::string &dest, std::string delimeter) ;
bool            check_valid_spaces(std::string &to_check, int authorized_space_number) ;
std::string     to_lower(std::string header) ;
ParseResult     check_repeated_key(std::vector<S_Header>& header) ;
bool            is_invalid_key_char(unsigned char c) ;
bool            is_invalid_value_char(unsigned char c) ;
bool            header_invalid_chars(const std::string& key, const std::string& value) ;
int             hex_char_to_int(char c) ;
int             hex_to_int(const std::string& hex_str) ;
ParseResult     match_server_location ( HttpRequest &request, std::string &response);

#endif