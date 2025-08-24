#include "HttpResponse.hpp"

bool ends_with(const std::string &str, const std::string &suffix) {
    if (suffix.size() > str.size()) return false;
    return std::equal(suffix.rbegin(), suffix.rend(), str.rbegin());
}


static std::string extension_to_mime(std::string ext) {
    std::string mime;
    size_t index = ext.find_last_of(".");
    if ( index != std::string::npos)
        ext.erase(0, index);
    if (ext == ".txt") mime = "text/plain";
    else if (ext == ".html") mime = "text/html";
    else if (ext == ".css") mime = "text/css";
    else if (ext == ".xml") mime = "text/xml";
    else if (ext == ".js") mime = "application/javascript";
    else if (ext == ".json") mime = "application/json";
    else if (ext == ".pdf") mime = "application/pdf";
    else if (ext == ".rtf") mime = "application/rtf";
    else if (ext == ".zip") mime = "application/zip";
    else if (ext == ".rar") mime = "application/x-rar-compressed";
    else if (ext == ".7z") mime = "application/x-7z-compressed";
    else if (ext == ".tar") mime = "application/x-tar";
    else if (ext == ".bz2") mime = "application/x-bzip2";
    else if (ext == ".gz") mime = "application/x-gzip";
    else if (ext == ".xz") mime = "application/x-xz";
    else if (ext == ".doc") mime = "application/msword";
    else if (ext == ".xls") mime = "application/vnd.ms-excel";
    else if (ext == ".ppt") mime = "application/vnd.ms-powerpoint";
    else if (ext == ".docx") mime = "application/vnd.openxmlformats-officedocument.wordprocessingml.document";
    else if (ext == ".xlsx") mime = "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet";
    else if (ext == ".pptx") mime = "application/vnd.openxmlformats-officedocument.presentationml.presentation";
    else if (ext == ".jar") mime = "application/java-archive";
    else if (ext == ".exe") mime = "application/x-msdownload";
    else if (ext == ".bin") mime = "application/octet-stream";
    else if (ext == ".png") mime = "image/png";
    else if (ext == ".jpeg") mime = "image/jpeg";
    else if (ext == ".jpg") mime = "image/jpg";
    else if (ext == ".gif") mime = "image/gif";
    else if (ext == ".webp") mime = "image/webp";
    else if (ext == ".ico") mime = "image/x-icon";
    else if (ext == ".bmp") mime = "image/bmp";
    else if (ext == ".svg") mime = "image/svg+xml";
    else if (ext == ".mp3") mime = "audio/mpeg";
    else if (ext == ".ogg") mime = "audio/ogg";
    else if (ext == ".wav") mime = "audio/wav";
    else if (ext == ".m4a") mime = "audio/x-m4a";
    else if (ext == ".mp4") mime = "video/mp4";
    else if (ext == ".webm") mime = "video/webm";
    else if (ext == ".avi") mime = "video/x-msvideo";
    else if (ext == ".mov") mime = "video/quicktime";
    else if (ext == ".flv") mime = "video/x-flv";
    else if (ext == ".swf") mime = "application/x-shockwave-flash";
    else if (ext == ".woff") mime = "application/x-font-woff";
    else if (ext == ".woff2") mime = "application/font-woff2";
    else if (ext == ".ttf") mime = "application/x-font-ttf";
    else if (ext == ".otf") mime = "font/otf";
    else if (ext == ".crt") mime = "application/x-x509-ca-cert";
    else if (ext == ".kml") mime = "application/vnd.google-earth.kml+xml";
    else if (ext == ".kmz") mime = "application/vnd.google-earth.kmz";
    else mime = "application/octet-stream";

    // Add charset=UTF-8 for text-based MIME types
    if (mime.rfind("text/", 0) == 0 ||
        mime == "application/javascript" ||
        mime == "application/json" ||
        mime == "application/xml" ||
        mime == "image/svg+xml" ||
        mime == "application/rtf") {
        mime += "; charset=UTF-8";
    }
    return mime;
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

std::string handle_redirection(const std::string& code, const std::string& url)
{
    std::string reason;
    if (code == "301") reason = "Moved Permanently";
    else if (code == "302") reason = "Found";
    else if (code == "303") reason = "See Other";
    else reason = "Redirect";

    std::string response;
    response = "HTTP/1.1 " + code + " " + reason + "\r\n";
    response += "Location: " + url + "\r\n";
    response += "Connection: close\r\n";
    response += "\r\n";
    return response;
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
        response = handle_redirection( CODE, URL ) ;
        return ;
    }
    ParseResult Pathresult = request.check_valid_path( );
    if ( Pathresult != OK)
    {
        if ( Pathresult == Redirect )
        {
            response = handle_redirection( "301", request.getUri() + '/' + request.getQuery() ) ;
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
    response += "Content-Type: " + ( request.getAutoIndex() == true ? "text/html; charset=UTF-8" : extension_to_mime(request.getPath()) ) + "\r\n";
    size_t len = body.size();
    response += "Content-Length: " + to_string(len) + "\r\n";
    response += "Connection: close\r\n";
    response += "\r\n";
    response += body; 
}
