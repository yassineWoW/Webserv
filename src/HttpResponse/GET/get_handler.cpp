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

void    handle_redirection(HttpRequest& request, std::string &response)
{
    const std::string CODE = request.getLocation().redirection_code;
    const std::string URL = request.getLocation().redirection_url;

    response = "HTTP/1.1 " + CODE + " Moved\r\n";
    response += "Content-Type: " + getResponseType( request.getPath() ) + "\r\n";
    response += "Location: " + URL + "\r\n";
    response += "Connection: close\r\n";
    response += "\r\n";
}

void    HttpResponse::handle_get(HttpRequest& request, std::string &response)
{

    ParseResult ServerResult = find_server_location( &request );
    
    if ( ServerResult != OK)
    {
        Errors errors;
        response = errors.handle_error( request.getServer().error_pages, ServerResult ) ;
    }
    else
    {
        if ( !request.getLocation().redirection_code.empty() )
        {
            handle_redirection( request, response ) ;
            return ;
        }
        ParseResult Pathresult = request.check_valid_path( );
        if ( Pathresult != OK)
        {   
            Errors errors;
            response = errors.handle_error( request.getServer().error_pages, Pathresult ) ;
        }
        else
        {
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
            response += "\r\n";
            response += body; 

        }
    }    
}
