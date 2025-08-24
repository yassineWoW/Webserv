#include "HttpResponse.hpp"

void return_error(HttpRequest& request, std::string &response, ParseResult result)
{
    Errors errors;
    response = errors.handle_error( request.getServer().error_pages, result ) ;  
}

void remove_file_directory(HttpRequest& request, std::string &response, std::string &path)
{
    std::string dirPath = request.getLocation().root;
    ParseResult Pathresult = isDirectoryAndAccessible( dirPath );
    if ( Pathresult != OK)
        return_error(request, response, Pathresult);
    else
    {
        std::string body = "";
        std::remove( path.c_str() );
        ParseResult Pathresult = isFileAndAccessible( request.getPath(), R_OK );
        if (Pathresult == NotFound)
        {
            body = "DELETE: file deleted successfully!\n";
            Pathresult = OK;
        }
        else
        {
            body = "DELETE: cannot delete the file.!";

            if (Pathresult == OK)
            Pathresult = InternalError;
        }
        response = create_res( Pathresult, body, ".html");
    }
}

void    HttpResponse::handle_delete(HttpRequest& request, std::string &response)
{
    std::string path = request.getPath();

    
    if ( !path.empty() && path[path.length() - 1] == '/' )
        path.erase( path.size() - 1 );

    ParseResult Pathresult = isDirectoryAndAccessible( path );
    if ( Pathresult != NotFound)
    {
        return_error(request, response, Forbidden);
        return ;
    }

    Pathresult = isFileAndAccessible( path, W_OK );

    if ( Pathresult != OK)
    {
        return_error(request, response, Pathresult);
        return ;
    }
    else
        remove_file_directory(request, response, path);
}
