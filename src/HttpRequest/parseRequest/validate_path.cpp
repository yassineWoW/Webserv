#include "HttpRequest.hpp"

ParseResult isDirectoryAndAccessible(const std::string &path)
{
    struct stat sb;
    if (stat(path.c_str(), &sb) != 0)
        return ( NotFound ) ;  // does not exist or error
    
    if (!S_ISDIR(sb.st_mode))
        return ( NotFound ) ;  // not a directory
    
    if (access(path.c_str(), R_OK | X_OK) != 0)
        return ( Forbidden ) ;  // no permission to read or enter
    
    return ( OK );
}

ParseResult isFileAndAccessible(const std::string &path, int accessFlag)
{
    struct stat sb;
    if (stat(path.c_str(), &sb) != 0)
        return ( NotFound ) ;  // does not exist or error
    
    if (!S_ISREG(sb.st_mode))
        return ( NotFound ) ;  // not a FILE
    
    if (access(path.c_str(), accessFlag) != 0)
        return ( Forbidden ) ;  // no permission to read or enter
    
    return ( OK );
}

ParseResult HttpRequest::check_valid_path()
{
    std::cout << "The path start!\n" << std::endl;
    ParseResult result = isDirectoryAndAccessible( this->path );
    if ( result == OK )
    {
        std::cout << " the path is a directory!\n" << std::endl;

        LocationConfig location = this->location;
        path = path + location.index;
        if ( isFileAndAccessible( path, R_OK ) == OK) {
            std::cout << "the target is a index file!!\n" << std::endl;
            return ( OK );
        } 
        else if ( location.autoindex == true )
        {
            // generateAutoindexHtml()
            std::cout << "call generateAutoindexHtml()!\n" << std::endl;
            return ( OK );
        }
        else
        {
            std::cout << "The path is not accessible (Forbidden).\n";
            return (Forbidden);
        }
        
        return ( OK );
    }
    else if (result == NotFound)
    {
        size_t len = this->path.length();
        if (len > 0 && this->path[len - 1] == '/')
            this->path.erase(len - 1, 1);
        result = isFileAndAccessible( this->path, R_OK ) ;
        if ( result == OK)
        {
            std::cout << " the path is a file!\n" << std::endl;
            return ( OK );
        }
        else {
            std::cout << " the path is nOT fOUND!\n" << std::endl;
            return ( result ) ;
        }
    }
    std::cout << "The path is not accessible (Forbidden).\n";
    return ( Forbidden ) ; 
}