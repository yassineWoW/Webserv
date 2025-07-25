// #include "HttpRequest.hpp"
#include "cfileparser.hpp"
#include "HttpResponse.hpp"

#include <cerrno>
#include <cstring>
#include <ctime>
#include <fstream>
#include <iostream>
#include <sstream>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>
#include <string>

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
    if (mime == "image/jpeg") return ".jpeg";
    if (mime == "image/jpg") return ".jpg";
    if (mime == "image/gif") return ".gif";
    if (mime == "application/pdf") return ".pdf";
    // size_t slash_pos = mime.find('/');
    // if (slash_pos != std::string::npos && slash_pos + 1 < mime.size()) {
    //     return "." + mime.substr(slash_pos + 1);
    // }
    return ".bin";
}

static std::string extract_content_type(const std::string& part) {
    size_t ct_pos = part.find("Content-Type:");
    if (ct_pos == std::string::npos) return "";
    size_t start = ct_pos + 13;
    while (start < part.size() && (part[start] == ' ' || part[start] == '\t')) ++start;
    size_t end = part.find("\r\n", start);
    if (end == std::string::npos) return "";
    return part.substr(start, end - start);
}

static std::string extract_body(const std::string& part) {
    size_t pos = part.find("\r\n\r\n");
    if (pos == std::string::npos) return "";
    return part.substr(pos + 4);
}

static bool ensure_directory(const std::string& path) {
    if (path.empty()) return false;
    struct stat st;
    if (stat(path.c_str(), &st) == 0) {
        if (S_ISDIR(st.st_mode)) return true;
        return false;
    }
    size_t slash = path.find_last_of('/');
    if (slash != std::string::npos && slash > 0) {
        if (!ensure_directory(path.substr(0, slash))) return false;
    }
    if (mkdir(path.c_str(), 0755) != 0 && errno != EEXIST) {
        std::cerr << "Failed to create directory: " << path << " (" << strerror(errno) << ")" << std::endl;
        if (errno == EACCES) {
            std::cerr << "Permission denied for directory: " << path << std::endl;
        }
        return false;
    }
    return true;
}

static ParseResult handle_multipart(const std::string& body, const std::string& boundary, std::vector<std::string>& stored_bodies, const std::string& upload_path) {
    std::string delimiter = "--" + boundary;
    std::string end_delimiter = delimiter + "--";
    size_t start = 0, end = 0, part_num = 0;
    bool found_part = false;

    if (!ensure_directory(upload_path)) {
        std::cerr << "Upload path does not exist or cannot be created: " << upload_path << std::endl;
        return InternalError;
    }

    while ((start = body.find(delimiter, end)) != std::string::npos) {
        start += delimiter.length();
        if (body.substr(start, 2) == "--") break;
        if (body.substr(start, 2) == "\r\n") start += 2;
        end = body.find(delimiter, start);
        std::string part = body.substr(start, end - start);

        std::string content_type = extract_content_type(part);
        std::string part_body = extract_body(part);

        if (part_body.empty()) continue;

        std::time_t rawTime;
        std::time(&rawTime);
        std::tm *timeInfo = std::localtime(&rawTime);

        std::ostringstream filenameStream;
        filenameStream << upload_path << "/part_" << part_num << "_";
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

        std::string extension = mime_to_extension(content_type);
        filenameStream << extension;
        std::string filename = filenameStream.str();

        std::ofstream ofs(filename.c_str(), std::ios::binary);
        if (!ofs.is_open()) {
            std::cerr << "Failed to open file for writing: " << filename << std::endl;
            return InternalError;
        }
        ofs.write(part_body.c_str(), part_body.size());
        ofs.close();

        stored_bodies.push_back(part_body);
        ++part_num;
        found_part = true;
    }
    return found_part ? OK : BadRequest;
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
    response << "------------Content-Type: text/plain\r\n";
    response << "\r\n";
    response << body;
    
    return response.str();
}


std::string HttpResponse::handle_post(HttpRequest& request, std::vector<std::string>& stored_bodies)
{
    const std::string& body = request.getBody();
    const std::string& content_type = request.getContentType();
    const std::string& upload_path = request.getLocation().upload_path;
    std::cout << "-----------------------------------------" << std::endl;
    std::cout << request.getLocation().upload_path << std::endl;
    std::cout << "Upload path: " << upload_path << std::endl;
    std::cout << "-----------------------------------------" << std::endl;

    if (body.empty())
        return HttpResponse::create_response(BadRequest, "POST body is empty.");

    if (upload_path.empty()) {
        std::cerr << "Upload path is empty in config." << std::endl;
        return HttpResponse::create_response(InternalError, "Upload path is not configured.");
    }

    if (!ensure_directory(upload_path)) {
        std::cerr << "Upload path does not exist or cannot be created: " << upload_path << std::endl;
        if (access(upload_path.c_str(), W_OK) != 0) {
            return HttpResponse::create_response(InternalError, "Upload path is not writable by server process. Please check directory permissions or choose a different upload_path in your config.");
        }
        return HttpResponse::create_response(InternalError, "Upload path does not exist or cannot be created.");
    }

    if (content_type.find("multipart/") == 0) {
        size_t bpos = content_type.find("boundary=");
        if (bpos == std::string::npos)
            return HttpResponse::create_response(BadRequest, "Missing boundary in multipart content-type.");
        std::string boundary = content_type.substr(bpos + 9);
        if (!boundary.empty() && boundary[0] == '"') {
            size_t endq = boundary.find('"', 1);
            boundary = boundary.substr(1, endq - 1);
        }
        ParseResult res = handle_multipart(body, boundary, stored_bodies, upload_path);
        if (res != OK)
            return HttpResponse::create_response(res, "Failed to process multipart POST.");
        return HttpResponse::create_response(OK, "Multipart POST stored in files.");
    }

    std::time_t rawTime;
    std::time(&rawTime);
    std::tm *timeInfo = std::localtime(&rawTime);

    std::ostringstream filenameStream;
    filenameStream << upload_path << "/file_";
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

    std::string extension = mime_to_extension(content_type);
    filenameStream << extension;
    std::string filename = filenameStream.str();

    std::ofstream outfile(filename.c_str(), std::ios::app | std::ios::binary);
    if (!outfile.is_open()) {
        std::cerr << "Failed to open file for writing: " << filename << std::endl;
        return HttpResponse::create_response(InternalError, "Failed to open file for writing.");
    }
    outfile.write(body.c_str(), body.size());
    if (!outfile) {
        std::cerr << "Failed to write body to file: " << filename << std::endl;
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
