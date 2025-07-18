// #include "HttpRequest.hpp"
#include "cfileparser.hpp"
#include "HttpResponse.hpp"

static std::map<ParseResult, std::pair<int, std::string> > create_status_map() 
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
    return m;
}

static std::string mime_to_extension(const std::string& mime) {
    if (mime == "text/plain") return ".txt";
    if (mime == "text/html") return ".html";
    if (mime == "application/json") return ".json";
    if (mime == "application/xml") return ".xml";
    if (mime == "image/png") return ".png";
    if (mime == "image/jpeg") return ".jpg";
    if (mime == "image/gif") return ".gif";
    // if (mime == "application/pdf") return ".pdf";
    size_t slash_pos = mime.find('/');
    if (slash_pos != std::string::npos && slash_pos + 1 < mime.size()) {
        return "." + mime.substr(slash_pos + 1);
    }
    return ".bin";
}

std::string HttpResponse::create_response(ParseResult code, const std::string& body)
{
    static std::map<ParseResult, std::pair<int, std::string> > status_map = create_status_map();
    
    std::ostringstream response;
    std::map<ParseResult, std::pair<int, std::string> >::iterator it = status_map.find(code);
    int status_code = 500;
    std::string status_text = "Internal Server Error";
    
    if (it != status_map.end()) {
        status_code = it->second.first;
        status_text = it->second.second;
    }
    
    response << "HTTP/1.1 " << status_code << " " << status_text << "\r\n";
    response << "Content-Length: " << body.length() << "\r\n";
    response << "Content-Type: text/plain\r\n";
    response << "\r\n";
    // response << body;
    
    return response.str();
}


std::string HttpResponse::handle_post(HttpRequest& request, std::vector<std::string>& stored_bodies)
{
    const std::string& body = request.getBody();   
    std::cout << "-----------------------------------------" << std::endl;
    std::cout << request.getContentType() << std::endl;
    std::cout << "-----------------------------------------" << std::endl;
    const size_t max_body_size = 1024 * 1024; // 1 MB
    std::time_t rawTime;
    std::time(&rawTime);
    std::tm *timeInfo = std::localtime(&rawTime);

    std::ostringstream filenameStream;
    filenameStream << "file_";
    filenameStream << (1900 + timeInfo->tm_year);
    if (timeInfo->tm_mon + 1 < 10) filenameStream << "0";
    filenameStream << (timeInfo->tm_mon + 1);
    if (timeInfo->tm_mday < 10) filenameStream << "0";
    filenameStream << timeInfo->tm_mday << "_";

    if (timeInfo->tm_hour < 10) filenameStream << "0";
    filenameStream << timeInfo->tm_hour;
    if (timeInfo->tm_min < 10) filenameStream << "0";
    filenameStream << timeInfo->tm_min;
    if (timeInfo->tm_sec < 10) filenameStream << "0";
    filenameStream << timeInfo->tm_sec;

    // Get extension from Content-Type
    std::string extension = mime_to_extension(request.getContentType());
    filenameStream << extension;

    std::string filename = filenameStream.str();
    
    if (body.empty())
        return HttpResponse::create_response(BadRequest, "POST body is empty.");

    if (body.size() > max_body_size)
        return HttpResponse::create_response(PayloadTooLarge, "POST body too large.");

    // Always create the file in the same folder as post_handler.cpp
    std::ofstream outfile(filename.c_str(), std::ios::app | std::ios::binary);
    if (!outfile.is_open()) {
        return HttpResponse::create_response(InternalError, "Failed to open file for writing.");
    }
    outfile.write(body.c_str(), body.size());
    if (!outfile) {
        return HttpResponse::create_response(InternalError, "Failed to write body to file.");
    }
    outfile.close();

    stored_bodies.push_back(body);

    return HttpResponse::create_response(OK, "POST body stored in file.");
}


// void HttpRequest::ftprint() {
//     std::cout << "Method: " << r_method << std::endl;
//     std::cout << "URL: " << r_url << std::endl;
//     std::cout << "Version: " << r_version << std::endl;
//     std::cout << "Query: " << r_query << std::endl;
    
//     std::cout << "Headers:" << std::endl;
//     for (std::vector<S_Header>::const_iterator it = r_header.begin(); it != r_header.end(); ++it) {
//         std::cout << it->key << ": " << it->value << std::endl;
//     }

//     std::cout << "Has Content-Length: " << (r_has_content_length ? "Yes" : "No") << std::endl;
//     std::cout << "Content-Length: " << r_content_length << std::endl;
//     std::cout << "Has Transfer-Encoding: " << (r_has_transfer_encoding ? "Yes" : "No") << std::endl;

//     std::cout << "Body: " << r_body << std::endl;
// }
