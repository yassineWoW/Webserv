// #include "HttpRequest.hpp"
#include "cfileparser.hpp"
#include "HttpResponse.hpp"
#include "multiplexer.hpp"

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

#include <fcntl.h>

static std::map<ParseResult, std::pair<int, std::string> > create_status_map() 
{
    std::map<ParseResult, std::pair<int, std::string> > m;
    m[OK]                     = std::make_pair(200, "OK");
    m[Incomplete]             = std::make_pair(100, "Continue");
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

static std::string mime_to_extension(const std::string& mime)
{
    if (mime == "text/plain") return ".txt";
    if (mime == "text/html") return ".html";
    if (mime == "text/css") return ".css";
    if (mime == "text/xml") return ".xml";
    if (mime == "text/javascript" || mime == "application/javascript") return ".js";
    if (mime == "application/json") return ".json";
    if (mime == "application/xml") return ".xml";
    if (mime == "application/pdf") return ".pdf";
    if (mime == "application/rtf") return ".rtf";
    if (mime == "application/zip") return ".zip";
    if (mime == "application/x-rar-compressed") return ".rar";
    if (mime == "application/x-7z-compressed") return ".7z";
    if (mime == "application/x-tar") return ".tar";
    if (mime == "application/x-bzip2") return ".bz2";
    if (mime == "application/x-gzip") return ".gz";
    if (mime == "application/x-xz") return ".xz";
    if (mime == "application/msword") return ".doc";
    if (mime == "application/vnd.ms-excel") return ".xls";
    if (mime == "application/vnd.ms-powerpoint") return ".ppt";
    if (mime == "application/vnd.openxmlformats-officedocument.wordprocessingml.document") return ".docx";
    if (mime == "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet") return ".xlsx";
    if (mime == "application/vnd.openxmlformats-officedocument.presentationml.presentation") return ".pptx";
    if (mime == "application/java-archive") return ".jar";
    if (mime == "application/x-msdownload") return ".exe";
    if (mime == "application/octet-stream") return ".bin";
    if (mime == "image/png") return ".png";
    if (mime == "image/jpeg") return ".jpeg";
    if (mime == "image/jpg") return ".jpg";
    if (mime == "image/gif") return ".gif";
    if (mime == "image/webp") return ".webp";
    if (mime == "image/x-icon") return ".ico";
    if (mime == "image/bmp") return ".bmp";
    if (mime == "image/svg+xml") return ".svg";
    if (mime == "audio/mpeg") return ".mp3";
    if (mime == "audio/ogg") return ".ogg";
    if (mime == "audio/wav") return ".wav";
    if (mime == "audio/x-m4a") return ".m4a";
    if (mime == "video/mp4") return ".mp4";
    if (mime == "video/webm") return ".webm";
    if (mime == "video/x-msvideo") return ".avi";
    if (mime == "video/quicktime") return ".mov";
    if (mime == "video/x-flv") return ".flv";
    if (mime == "application/x-shockwave-flash") return ".swf";
    if (mime == "application/x-font-woff") return ".woff";
    if (mime == "application/font-woff2") return ".woff2";
    if (mime == "application/x-font-ttf" || mime == "font/ttf") return ".ttf";
    if (mime == "font/otf") return ".otf";
    if (mime == "application/x-x509-ca-cert") return ".crt";
    if (mime == "application/vnd.google-earth.kml+xml") return ".kml";
    if (mime == "application/vnd.google-earth.kmz") return ".kmz";
    return ".txt";
}

static std::string extract_content_type(const std::string& part)
{
    size_t ct_pos = part.find("Content-Type:");
    if (ct_pos == std::string::npos) return "";
    size_t start = ct_pos + 13;
    while (start < part.size() && (part[start] == ' ' || part[start] == '\t')) ++start;
    size_t end = part.find("\r\n", start);
    if (end == std::string::npos) return "";
    return part.substr(start, end - start);
}

static std::string extract_body(const std::string& part)
{
    size_t pos = part.find("\r\n\r\n");
    if (pos == std::string::npos) return "";
    return part.substr(pos + 4);
}

static bool ensure_directory(const std::string& path)
{
    if (path.empty()) return false;
    struct stat st;
    if (stat(path.c_str(), &st) == 0)
    {
        if (S_ISDIR(st.st_mode)) return true;
        return false;
    }
    size_t slash = path.find_last_of('/');
    if (slash != std::string::npos && slash > 0)
        if (!ensure_directory(path.substr(0, slash))) return false;
    if (mkdir(path.c_str(), 0755) != 0 && errno != EEXIST)
    {
        std::cerr << "Failed to create directory: " << path << " (" << strerror(errno) << ")" << std::endl;
        if (errno == EACCES)
            std::cerr << "Permission denied for directory: " << path << std::endl;
        return false;
    }
    return true;
}

static ParseResult handle_multipart(const std::string& body, const std::string& boundary, std::vector<std::string>& stored_bodies, const std::string& upload_path)
{
    std::string delimiter = "--" + boundary;
    std::string end_delimiter = delimiter + "--";
    size_t start = 0, end = 0, part_num = 0;
    bool found_part = false;

    if (!ensure_directory(upload_path))
    {
        std::cerr << "Upload path does not exist or cannot be created: " << upload_path << std::endl;
        return InternalError;
    }

    while ((start = body.find(delimiter, end)) != std::string::npos)
    {
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
        if (!ofs.is_open())
        {
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

    if (body.empty())
        return HttpResponse::create_response(BadRequest, "POST body is empty.");

    if (upload_path.empty())
    {
        std::cerr << "Upload path is empty in config." << std::endl;
        return HttpResponse::create_response(InternalError, "Upload path is not configured.");
    }

    if (!ensure_directory(upload_path))
    {
        std::cerr << "Upload path does not exist or cannot be created: " << upload_path << std::endl;
        if (access(upload_path.c_str(), W_OK) != 0)
            return HttpResponse::create_response(InternalError, "Upload path is not writable by server process. Please check directory permissions or choose a different upload_path in your config.");
        return HttpResponse::create_response(InternalError, "Upload path does not exist or cannot be created.");
    }

    // Check if POST body size exceeds config limit
    size_t max_body_size = request.getServer().client_max_body_size;
    // std::cout << "This is max body size : " << max_body_size << std::endl;
    // std::cout << "This is max body.size() : " << body.size() << std::endl;

    // exit(11);
    if (body.size() > max_body_size)
        return HttpResponse::create_response(PayloadTooLarge, "POST body exceeds maximum allowed size.");

    // Check if upload_path is a file and accessible (should be a directory, but if file exists, error)
    // isFileAndAccessible(upload_path, W_OK);
    // std::cout << "This is fileCheck : " << isFileAndAccessible(upload_path, W_OK) << std::endl;
    // exit(1);
    // if (fileCheck == OK)
    //     return HttpResponse::create_response(Forbidden, "Upload path is a file, not a directory.");

    if (content_type.find("multipart/") == 0)
    {
        size_t bpos = content_type.find("boundary=");
        if (bpos == std::string::npos)
            return HttpResponse::create_response(BadRequest, "Missing boundary in multipart content-type.");
        std::string boundary = content_type.substr(bpos + 9);
        if (!boundary.empty() && boundary[0] == '"')
        {
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
    if (!outfile.is_open())
    {
        std::cerr << "Failed to open file for writing: " << filename << std::endl;
        return HttpResponse::create_response(InternalError, "Failed to open file for writing.");
    }
    outfile.write(body.c_str(), body.size());
    if (!outfile)
    {
        std::cerr << "Failed to write body to file: " << filename << std::endl;
        return HttpResponse::create_response(InternalError, "Failed to write body to file.");
    }
    outfile.close();

    stored_bodies.push_back(body);

    return HttpResponse::create_response(OK, "POST body stored in file.");
}
