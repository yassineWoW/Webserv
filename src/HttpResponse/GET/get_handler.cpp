#include "HttpResponse.hpp"

bool ends_with(const std::string &str, const std::string &suffix) {
    if (suffix.size() > str.size()) return false;
    return std::equal(suffix.rbegin(), suffix.rend(), str.rbegin());
}


std::string getResponseType(const std::string& path) {
    if (ends_with(path, ".html")) return "text/html";
    if (ends_with(path, ".css")) return "text/css";
    if (ends_with(path, ".js")) return "application/javascript";
    if (ends_with(path, ".jpg")) return "image/jpeg";
    if (ends_with(path, ".png")) return "image/png";
    return "application/octet-stream";
}


void    HttpResponse::handle_get(HttpRequest& request, std::string &response)
{

    ParseResult ServerResult = find_server_location( &request );
    ParseResult Pathresult = request.check_valid_path( );

    if ( ServerResult != OK)
    {
        response = HttpResponse::create_response( ServerResult, to_string( ServerResult ) );
    }
    else if ( Pathresult != OK)
    {        
        response = HttpResponse::create_response( Pathresult, to_string( Pathresult ) );
    }
    else
    {
        std::ifstream file(request.getPath().c_str(), std::ios::binary);
        std::ostringstream contentStream;
        contentStream << file.rdbuf();
        std::string body = contentStream.str();


        response = "HTTP/1.1 200 OK\r\n";
        response += "Content-Type: " + getResponseType(request.getPath()) + "\r\n";
        size_t len = body.size();
        response += "Content-Length: " + to_string(len) + "\r\n";
        response += "Connection: close\r\n";
        response += "\r\n";
        response += body;
    }
}
