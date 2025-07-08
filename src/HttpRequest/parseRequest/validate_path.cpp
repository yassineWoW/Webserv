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
    struct stat sb;
    std::cout << "The path start!\n" << std::endl;
    if ( stat(this->path.c_str(), &sb) == 0 && sb.st_mode & S_IFDIR) // is directory
    {
        LocationConfig location = this->location;
        
        if (location.autoindex == true )
        {
            path = path + location.index;

            if (location.index.empty() || !(stat(this->path.c_str(), &sb) == 0 && S_ISREG(sb.st_mode))) {
                // generateAutoindexHtml()
                std::cout << "call generateAutoindexHtml!\n" << std::endl;
            }
            else
            {
                if ( stat(this->path.c_str(), &sb) == 0 && !(sb.st_mode & S_IFDIR)) // is file
                {
                    std::cout << " the target is a index file!\n" << std::endl;
                }
            }
        }
        else
            return (Forbidden);
    }
    else
    {
        size_t len = this->path.length() - 1;
        if (this->path[len] == '/')
            this->path.erase(len, 1);
        if ( stat(this->path.c_str(), &sb) == 0) // is file
        {
            std::cout << " the target is a file!\n" << std::endl;
            
        }
        else {
            std::cout << "The Path is invalid!\n" << std::endl;
            return (NotFound);
        }
    }
    std::cout << "The Path end!\n" << std::endl;
    return (OK);
}