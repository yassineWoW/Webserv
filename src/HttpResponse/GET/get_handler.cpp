#include "HttpResponse.hpp"

bool ends_with(const std::string &str, const std::string &suffix) {
    if (suffix.size() > str.size()) return false;
    return std::equal(suffix.rbegin(), suffix.rend(), str.rbegin());
}


static std::string getResponseType( const std::string& path ) {
    if (ends_with(path, ".html")) return "text/html";
    if (ends_with(path, ".css")) return "text/css";
    if (ends_with(path, ".js")) return "application/javascript";
    if (ends_with(path, ".jpg")) return "image/jpeg";
    if (ends_with(path, ".png")) return "image/png";
    return "text/html";
}


void handle_auto_index_response(HttpRequest& request, std::string &body)
{
    std::vector<std::string>& files = request.getAutoIndexFiles();

    std::string html = "<!DOCTYPE html><html><head><title>Index of " + request.getUri() + 
                       "</title></head><body><h1>Index of " + request.getUri() + "</h1><ul>";

    for (size_t i = 0; i < files.size(); ++i) {
        body += "<li><a href=\"" + request.getUri() + files[i] + "\">" + files[i] + "</a></li>";
    }

    body += "</ul></body></html>";
}

std::string    handle_redirection(HttpRequest& request, std::string CODE, std::string URL)
{
    std::string response = "";

    response = "HTTP/1.1 " + CODE + " Moved\r\n";
    response += "Content-Type: " + getResponseType( request.getPath() ) + "\r\n";
    response += "Location: " + URL + "\r\n";
    response += "Connection: close\r\n";
    response += "\r\n";
    return ( response );
}

std::string setSessionId()
{
    static bool seeded = false;
    if (!seeded) {
        std::srand(static_cast<unsigned int>(std::time(0)));
        seeded = true;
    }

    std::ostringstream oss;

    for (int i = 0; i < 16; ++i) {
        int r = std::rand() % 256;
        oss << std::hex << std::setw(2) << std::setfill('0') << r;
    }

    return oss.str();
}

void    HttpResponse::handle_get(HttpRequest& request, std::string &response)
{
    if ( !request.getLocation().redirection_code.empty() )
    {
        const std::string CODE = request.getLocation().redirection_code;
        const std::string URL = request.getLocation().redirection_url;
        handle_redirection( request, CODE, URL ) ;
        return ;
    }
    ParseResult Pathresult = request.check_valid_path( );
    if ( Pathresult != OK)
    {
        if ( Pathresult == Redirect )
        {
            response = handle_redirection( request, "301", request.getUri() + '/' + request.getQuery() ) ;
            return ;
        }
        Errors errors;
        response = errors.handle_error( request.getServer().error_pages, Pathresult ) ;
        return ;
    }
    std::string body = "";
    if ( request.getAutoIndex() == true )
    {
        handle_auto_index_response( request, body );
    }
    else
    {
        std::ifstream file(request.getPath().c_str(), std::ios::binary);
        std::ostringstream contentStream;
        contentStream << file.rdbuf();
        body = contentStream.str();
    }
    response = "HTTP/1.1 200 OK\r\n";
    response += "Content-Type: " + getResponseType(request.getPath()) + "\r\n";
    size_t len = body.size();
    response += "Content-Length: " + to_string(len) + "\r\n";
    response += "Connection: close\r\n";
    if ( request.getCookies().find("sid") ==  request.getCookies().end())
    {
        response += "Set-Cookie: sid=" + setSessionId() + "; Secure; HttpOnly;\r\n";
    }
    response += "\r\n";
    response += body; 
}
