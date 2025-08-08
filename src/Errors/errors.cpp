#include "Errors.hpp"


static std::string getResponseType( const std::string& path ) {
    if (ends_with(path, ".html")) return "text/html";
    if (ends_with(path, ".css")) return "text/css";
    if (ends_with(path, ".txt")) return "text/txt";
    if (ends_with(path, ".js")) return "application/javascript";
    if (ends_with(path, ".jpg")) return "image/jpeg";
    if (ends_with(path, ".png")) return "image/png";
    return "text/html";
}

std::map<ParseResult, std::pair<int, std::string> > create_errors_map() 
{
    std::map<ParseResult, std::pair<int, std::string> > m;
    m[OK]                     = std::make_pair(200, "OK");
    m[Incomplete]             = std::make_pair(100, "Continue"); // optional
    m[BadRequest]             = std::make_pair(400, "Bad Request");
    m[NotAllowed]             = std::make_pair(405, "Method Not Allowed");
    m[LengthRequired]         = std::make_pair(411, "Length Required");
    m[PayloadTooLarge]        = std::make_pair(413, "Payload Too Large");
    m[URITooLong]             = std::make_pair(414, "URI Too Long");
    m[UnsupportedMediaType]   = std::make_pair(415, "Unsupported Media Type");
    m[HeaderFieldsTooLarge]   = std::make_pair(431, "Request Header Fields Too Large");
    m[NotFound]               = std::make_pair(404, "Not Found");
    m[Forbidden]              = std::make_pair(403, "Forbidden");
    m[Gone]                   = std::make_pair(410, "Gone");
    m[Conflict]               = std::make_pair(409, "Conflict");
    m[RequestTimeout]         = std::make_pair(408, "Request Timeout");
    m[HTTPVersionNotSupported]= std::make_pair(505, "HTTP Version Not Supported");
    m[InternalError]          = std::make_pair(500, "Internal Server Error");
    m[NotImplemented]         = std::make_pair(501, "Method Not Implemented");
    return m;
}


std::string create_res(ParseResult code, std::string body, std::string path)
{
    static std::map<ParseResult, std::pair<int, std::string> > status_map = create_errors_map();
    
    std::ostringstream response;
    std::map<ParseResult, std::pair<int, std::string> >::iterator it = status_map.find(code);
    int status_code = 500;
    std::string status_text = "Internal Server Error";
    
    if (it != status_map.end()) {
        status_code = it->second.first;
        status_text = it->second.second;
    }
    
    if (body.empty())
    {
        body = "<!DOCTYPE html>"
        "<html lang='en'>"
        "<head>"
        "<style>h1 {"
        "color: black;"
        "font-family: 'Trebuchet MS', 'Lucida Sans Unicode', 'Lucida Grande', 'Lucida Sans', Arial, sans-serif;"
        "margin-top: 50px;"
        "text-align: center;"
        "font-size: 35px;"
        "}</style>"
        "</head>"
        "<body><h1>";
        body += status_text;
        body += "</h1></body></html>";
    }

    response << "HTTP/1.1 " << status_code << " " << status_text << "\r\n";
    response << "Content-Length: " << body.length() << "\r\n";
    response << "Content-Type: " << getResponseType(path) << "\r\n";
    response << "\r\n";
    response << body;

    return response.str();
}


std::string Errors::handle_error ( std::map<int, std::string> error_pages, ParseResult status ) 
{
    std::string response = "";
    std::string body = "";
    std::map<int, std::string>::iterator error_page = error_pages.find(status);
        
    if (error_page != error_pages.end()) 
    {
        if (isFileAndAccessible( error_page->second , R_OK) == OK)
        {
            std::ifstream file(error_page->second .c_str(), std::ios::binary);
            std::ostringstream contentStream;
            contentStream << file.rdbuf();
            body = contentStream.str();
            response = create_res( status, body, error_page->second);
        }
        else
        {
            response = create_res( status, body, ".html");
        }
    }
    else
        response = create_res( status, body, ".html");
    std::cout << response <<std::endl;
    return ( response );
};

