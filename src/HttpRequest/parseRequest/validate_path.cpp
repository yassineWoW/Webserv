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

ParseResult HttpRequest::generateAutoindexHtml ( ){
    struct dirent *de;

    r_auto_index = true;
    DIR *dr = opendir( path.c_str( ) );

    if (dr == NULL)
        return ( Forbidden );

    while ( (de = readdir(dr)) != NULL )
    {
        r_auto_index_files.push_back( de->d_name );
    }
    closedir(dr);    
    return ( OK );
}

ParseResult HttpRequest::check_valid_path()
{
    ParseResult result = isDirectoryAndAccessible( this->path );
    if ( result == OK )
    {
        LocationConfig location = this->location;
        path = path + location.index;
        if ( isFileAndAccessible( path, R_OK ) == OK) {
            return ( OK );
        }

        else if (isFileAndAccessible( path, R_OK ) == Forbidden)
            return (Forbidden);
    
        else if ( location.autoindex == true && location.index.empty() )
        {
            std::cout << path <<std::endl;
            return ( generateAutoindexHtml() );
        }  

        return ( NotFound );
    }

    else if (result == NotFound)
    {
        size_t len = this->path.length();
        if (len > 0 && this->path[len - 1] == '/')
            this->path.erase(len - 1, 1);
        result = isFileAndAccessible( this->path, R_OK ) ;
        if ( result == OK)
        {
            return ( OK );
        }
        else {
            return ( result ) ;
        }
    }
    
    return ( Forbidden ) ; 
}