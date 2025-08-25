#include "HttpRequest.hpp"

ParseResult isDirectory(const std::string &path)
{
    struct stat sb;
    if (stat(path.c_str(), &sb) != 0)
        return ( NotFound ) ;
    
    if (!S_ISDIR(sb.st_mode))
        return ( NotFound ) ;
    
    return ( OK );
}

ParseResult isDirectoryAndAccessible(const std::string &path)
{
    struct stat sb;
    if (stat(path.c_str(), &sb) != 0)
        return ( NotFound ) ;
    
    if (!S_ISDIR(sb.st_mode))
        return ( NotFound ) ;
    
    if (access(path.c_str(), R_OK | X_OK) != 0)
        return ( Forbidden ) ;
    
    return ( OK );
}

ParseResult isFileAndAccessible(const std::string &path, int accessFlag)
{
    struct stat sb;
    if (stat(path.c_str(), &sb) != 0)
        return ( NotFound ) ;
    
    if (!S_ISREG(sb.st_mode))
        return ( NotFound ) ;
    
    if (access(path.c_str(), accessFlag) != 0)
        return ( Forbidden ) ;
    
    return ( OK );
}

ParseResult HttpRequest::generateAutoindexHtml ( ){
    struct dirent *de;

    r_auto_index = true;
    DIR *dr = opendir( path.c_str( ) );

    if (dr == NULL)
    {
        return ( Forbidden );
    }

    while ( (de = readdir(dr)) != NULL )
    {
        r_auto_index_files.push_back( de->d_name );
    }
    closedir(dr);    
    return ( OK );
}

ParseResult HttpRequest::check_valid_path()
{
    if (!r_url.empty() && r_url[r_url.length() - 1] != '/' && isDirectory(path) == OK)
    {
        return Redirect; 
    }

    ParseResult result = isDirectoryAndAccessible(path);

    if (result == OK)
    {
        LocationConfig location = this->location;
        std::string index_path = this->path + location.index;

        if (isFileAndAccessible(index_path, R_OK) == OK)
        {
            this->path = index_path;
            return OK;
        }

        else if (isFileAndAccessible(index_path, R_OK) == Forbidden)
            return Forbidden;

        else if (location.autoindex == true)
        {
            r_auto_index = true;
            return generateAutoindexHtml();
        }
        return NotFound;
    }
    else
        result = isFileAndAccessible(path, R_OK);
    return ( result );
}