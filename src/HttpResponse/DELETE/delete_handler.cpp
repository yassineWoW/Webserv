#include "HttpResponse.hpp"


void    HttpResponse::handle_delete(HttpRequest& request, std::string &response)
{
    ParseResult ServerResult = find_server_location( &request );
    
    if ( ServerResult != OK)
    {
        Errors errors;
        response = errors.handle_error( request.getServer().error_pages, ServerResult ) ;
    }
    else
    {
        std::string path = request.getPath();
        if ( !path.empty() && path[path.length() - 1] == '/' )
            path.erase( path.size() - 1 );

        ParseResult Pathresult = isDirectoryAndAccessible( path );
        if ( Pathresult != NotFound)
        {
            Errors errors;
            response = errors.handle_error( request.getServer().error_pages, Forbidden ) ;
            return ;
        }

        Pathresult = isFileAndAccessible( path, W_OK );

        if ( Pathresult != OK)
        {
            Errors errors;
            response = errors.handle_error( request.getServer().error_pages, Pathresult ) ;
            return ;
        }
        else
        {
            std::string dirPath = request.getLocation().root;
            ParseResult Pathresult = isDirectoryAndAccessible( dirPath );
            if ( Pathresult != OK)
            {
                Errors errors;
                response = errors.handle_error( request.getServer().error_pages, Pathresult ) ;
            }
            else
            {
                std::string body = "";
                std::remove( path.c_str() );
                ParseResult Pathresult = isFileAndAccessible( request.getPath(), R_OK );
                if (Pathresult == NotFound)
                {
                    body = "DELETE: file deleted successfully!";
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
    }    
}
