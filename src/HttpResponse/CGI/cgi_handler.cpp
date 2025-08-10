#include "Cgi.hpp"
#include "includes.hpp"
#include "HttpResponse.hpp"

// std::pair<int, pid_t> CGI_handler::execute_cgi_script(const std::string& script_path, const std::string& interpreter, HttpRequest& request) {
//     // First check if the script file exists
//     if (access(script_path.c_str(), F_OK) != 0) {
//         std::cerr << "CGI script not found: " << script_path << std::endl;
//         return std::make_pair(-1, -1);
//     }
    
//     // Check if interpreter exists
//     if (access(interpreter.c_str(), X_OK) != 0) {
//         std::cerr << "CGI interpreter not found or not executable: " << interpreter << std::endl;
//         return std::make_pair(-1, -1);
//     }

//     int pipefd[2];
//     if (pipe(pipefd) == -1) {
//         perror("pipe failed for CGI");
//         return std::make_pair(-1, -1);
//     }

//     pid_t pid = fork();
//     if (pid < 0) {
//         close(pipefd[0]);
//         close(pipefd[1]);
//         perror("fork failed for CGI");
//         return std::make_pair(-1, -1);
//     }

//     if (pid == 0) {
//         // Child process
//         dup2(pipefd[1], STDOUT_FILENO);
//         dup2(pipefd[1], STDERR_FILENO);
//         close(pipefd[0]);
//         close(pipefd[1]);

//         // Set up environment variables
//         std::vector<std::string> env;
//         std::stringstream ss;
//         ss << request.getContentLength();
//         env.push_back("CONTENT_LENGTH=" + ss.str());
//         env.push_back("REQUEST_METHOD=" + request.getMethod());
//         env.push_back("QUERY_STRING=" + request.getQuery());
//         env.push_back("CONTENT_TYPE=" + request.getContentType());
//         env.push_back("REDIRECT_STATUS=200");
//         env.push_back("SCRIPT_FILENAME=" + script_path);
//         env.push_back("SCRIPT_NAME=" + request.getUri());
//         env.push_back("DOCUMENT_ROOT=" + request.getLocation().root);
//         env.push_back("SERVER_NAME=localhost");
//         env.push_back("SERVER_PORT=8080");
//         env.push_back("PATH_INFO=");
//         env.push_back("PATH_TRANSLATED=" + script_path);

//         std::map<std::string, std::string>& cookies = request.getCookies();
//         if (!cookies.empty()) {
//             std::string cookieStr;
//             for (std::map<std::string, std::string>::iterator it = cookies.begin(); it != cookies.end(); ++it) {
//                 if (!cookieStr.empty())
//                     cookieStr += "; ";
//                 cookieStr += it->first + "=" + it->second;
//             }
//             env.push_back("HTTP_COOKIE=" + cookieStr);
//         }

//         // Convert to char* array for execve
//         std::vector<char*> envp;
//         for (size_t i = 0; i < env.size(); ++i) {
//             envp.push_back(strdup(env[i].c_str()));
//         }
//         envp.push_back(NULL);

//         // Prepare arguments
//         char* const argv[] = {
//             strdup(interpreter.c_str()),
//             strdup(script_path.c_str()),
//             NULL
//         };

//         execve(interpreter.c_str(), argv, &envp[0]);
//         perror("execve failed");
//         _exit(1);
//     }

//     // Parent process
//     close(pipefd[1]); // Close write end
    
//     // Make read end non-blocking
//     int flags = fcntl(pipefd[0], F_GETFL, 0);
//     if (flags < 0 || fcntl(pipefd[0], F_SETFL, flags | O_NONBLOCK) < 0) {
//         perror("fcntl failed for CGI pipe");
//         close(pipefd[0]);
//         return std::make_pair(-1, -1);
//     }

//     return std::make_pair(pipefd[0], pid); // Return both read end of pipe and PID
// }

// Overload for POST: returns stdout fd, pid, and sets stdin_fd_out
std::pair<int, pid_t> CGI_handler::execute_cgi_script(const std::string& script_path, const std::string& interpreter, HttpRequest& request, bool is_post, int &stdin_fd_out) {
    // Debug print
    std::cerr << "[CGI] Interpreter: " << interpreter << std::endl;
    std::cerr << "[CGI] Script path: " << script_path << std::endl;

    // First check if the script file exists
    if (access(script_path.c_str(), F_OK) != 0) {
        std::cerr << "[CGI] CGI script not found: " << script_path << std::endl;
        return std::make_pair(-1, -1);
    }
    if (access(interpreter.c_str(), X_OK) != 0) {
        std::cerr << "[CGI] CGI interpreter not found or not executable: " << interpreter << std::endl;
        return std::make_pair(-1, -1);
    }

    int pipefd[2]; // stdout
    int stdin_pipe[2]; // stdin
    if (pipe(pipefd) == -1)
        return std::make_pair(-1, -1);
    if (is_post && pipe(stdin_pipe) == -1) {
        close(pipefd[0]); close(pipefd[1]);
        return std::make_pair(-1, -1);
    }
    pid_t pid = fork();
    if (pid < 0) {
        close(pipefd[0]); close(pipefd[1]);
        if (is_post) { close(stdin_pipe[0]); close(stdin_pipe[1]); }
        return std::make_pair(-1, -1);
    }
    if (pid == 0) {
        dup2(pipefd[1], STDOUT_FILENO);
        dup2(pipefd[1], STDERR_FILENO);
        close(pipefd[0]); close(pipefd[1]);
        if (is_post) {
            dup2(stdin_pipe[0], STDIN_FILENO);
            close(stdin_pipe[0]); close(stdin_pipe[1]);
        }

        // Set up environment variables
        std::vector<std::string> env;
        std::stringstream ss;
        ss << request.getContentLength();
        env.push_back("CONTENT_LENGTH=" + ss.str());
        env.push_back("REQUEST_METHOD=" + request.getMethod());
        env.push_back("QUERY_STRING=" + request.getQuery());
        env.push_back("CONTENT_TYPE=" + request.getContentType());
        env.push_back("REDIRECT_STATUS=200");
        env.push_back("SCRIPT_FILENAME=" + script_path);
        env.push_back("SCRIPT_NAME=" + request.getUri());
        env.push_back("DOCUMENT_ROOT=" + request.getLocation().root);
        env.push_back("SERVER_NAME=localhost");
        env.push_back("SERVER_PORT=8080");
        env.push_back("PATH_INFO=");
        env.push_back("PATH_TRANSLATED=" + script_path);

        std::map<std::string, std::string>& cookies = request.getCookies();
        if (!cookies.empty()) {
            std::string cookieStr;
            for (std::map<std::string, std::string>::iterator it = cookies.begin(); it != cookies.end(); ++it) {
                if (!cookieStr.empty())
                    cookieStr += "; ";
                cookieStr += it->first + "=" + it->second;
            }
            env.push_back("HTTP_COOKIE=" + cookieStr);
        }

        // Convert to char* array for execve
        std::vector<char*> envp;
        for (size_t i = 0; i < env.size(); ++i) {
            envp.push_back(strdup(env[i].c_str()));
        }
        envp.push_back(NULL);

        // Prepare arguments
        char* const argv[] = {
            strdup(interpreter.c_str()),
            strdup(script_path.c_str()),
            NULL
        };

        execve(interpreter.c_str(), argv, &envp[0]);
        perror("execve failed");
        _exit(1);
    }
    close(pipefd[1]);
    if (is_post) {
        close(stdin_pipe[0]);
        stdin_fd_out = stdin_pipe[1];
        int flags = fcntl(stdin_fd_out, F_GETFL, 0);
        if (flags >= 0) fcntl(stdin_fd_out, F_SETFL, flags | O_NONBLOCK);
    } else {
        stdin_fd_out = -1;
    }
    int flags = fcntl(pipefd[0], F_GETFL, 0);
    if (flags >= 0) fcntl(pipefd[0], F_SETFL, flags | O_NONBLOCK);
    return std::make_pair(pipefd[0], pid);
}

// POST/GET selector
std::string CGI_handler::handle_cgi(HttpRequest &request, bool is_post)
{
    std::string url = request.getUri();
    std::string script_path = request.getPath();
    
    std::cout << "CGI Debug - URL                       : " << url << std::endl;
    std::cout << "CGI Debug - Script path               : " << script_path << std::endl;

    if (url.size() >= 4 && url.substr(url.size() - 4) == ".php") {
        // Try different PHP interpreter paths
        std::string php_interpreter = "/usr/bin/php-cgi";

        if (php_interpreter.empty()) {
            std::cerr << "No PHP interpreter found!" << std::endl;
            return HttpResponse::create_response(InternalError, "PHP interpreter not found on server");
        }
        
        std::cout << "Using PHP interpreter: " << php_interpreter << std::endl;
        
        int stdin_fd = -1;
        std::pair<int, pid_t> cgi_result = execute_cgi_script(script_path, php_interpreter, request, is_post, stdin_fd);
        if (cgi_result.first < 0) {
            return HttpResponse::create_response(InternalError, "Failed to start CGI process pp");
        }
        if (is_post)
            return "CGI_STARTED:" + to_string(cgi_result.first) + ":" + to_string(cgi_result.second) + ":" + to_string(stdin_fd);
        else
            return "CGI_STARTED:" + to_string(cgi_result.first) + ":" + to_string(cgi_result.second);
    }
    
    if (url.size() >= 3 && url.substr(url.size() - 3) == ".py") {
        std::vector<std::string> python_paths;
        python_paths.push_back("/usr/bin/python3");
        python_paths.push_back("/usr/bin/python");
        python_paths.push_back("/usr/local/bin/python3");
        
        std::string python_interpreter;
        for (size_t i = 0; i < python_paths.size(); ++i) {
            if (access(python_paths[i].c_str(), X_OK) == 0) {
                python_interpreter = python_paths[i];
                break;
            }
        }
        
        if (python_interpreter.empty()) {
            return HttpResponse::create_response(InternalError, "Python interpreter not found on server");
        }
        
        int stdin_fd = -1;
        std::pair<int, pid_t> cgi_result = execute_cgi_script(script_path, python_interpreter, request, is_post, stdin_fd);
        if (cgi_result.first < 0) {
            return HttpResponse::create_response(InternalError, "Failed to start CGI process");
        }
        if (is_post)
            return "CGI_STARTED:" + to_string(cgi_result.first) + ":" + to_string(cgi_result.second) + ":" + to_string(stdin_fd);
        else
            return "CGI_STARTED:" + to_string(cgi_result.first) + ":" + to_string(cgi_result.second);
    }
    
    return HttpResponse::create_response(InternalError, "<h1>CGI not supported for this file type</h1>");
}


