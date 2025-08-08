#include "Cgi.hpp"
#include "includes.hpp"
#include "HttpResponse.hpp"


std::string CGI_handler::execute_cgi_script(const std::string& script_path, const std::string& interpreter, HttpRequest& request) {
    int pipefd[2];
    if (pipe(pipefd) == -1)
        return "CGI pipe error\n";

    pid_t pid = fork();
    if (pid < 0) {
        close(pipefd[0]);
        close(pipefd[1]);
        return "CGI fork error\n";
    }

    if (pid == 0) {

        dup2(pipefd[1], STDOUT_FILENO);
        dup2(pipefd[1], STDERR_FILENO);
        close(pipefd[0]);
        close(pipefd[1]);

        char* const argv[] = {
            strdup(interpreter.c_str()),
            strdup(script_path.c_str()),
            NULL
        };
        std::vector<std::string> env;
        std::stringstream ss;
        ss << request.getContentLength();
        env.push_back("CONTENT_LENGTH=" + ss.str());
        env.push_back("REQUEST_METHOD=" + request.getMethod());
        env.push_back("QUERY_STRING=" + request.getQuery());
        env.push_back("CONTENT_LENGTH=" + ss.str());
        env.push_back("CONTENT_TYPE=" + request.getContentType());
        env.push_back("REDIRECT_STATUS=200");
        env.push_back("SCRIPT_FILENAME=" + script_path);
        env.push_back("SCRIPT_NAME=" + script_path);
        env.push_back("DOCUMENT_ROOT=" + request.getLocation().root);
        env.push_back("PHP_SELF=" + script_path);
        env.push_back("PATH_TRANSLATED=" + script_path);

        std::map<std::string, std::string>& cookies = request.getCookies();
        if (!cookies.empty()) {
            std::string cookieStr;
            for (std::map<std::string, std::string>::iterator it = cookies.begin(); it != cookies.end(); ++it) {
                if (!cookieStr.empty())
                    cookieStr += "; ";
                cookieStr += it->first + ": " + it->second;
            }
            std::cout << "[DEBUG] HTTP_COOKIE = " << cookieStr << std::endl;
            env.push_back("HTTP_COOKIE=" + cookieStr);
        }

        std::vector<char*> envp;
        for (size_t i = 0; i < env.size(); ++i) {
            envp.push_back(strdup(env[i].c_str()));
        }
        envp.push_back(NULL);

        execve(interpreter.c_str(), argv, &envp[0]);

        perror("execve failed");
        _exit(1);
    }

    close(pipefd[1]);
    std::string output;
    char buffer[4096];
    ssize_t n;
    while ((n = read(pipefd[0], buffer, sizeof(buffer))) > 0) {
        output.append(buffer, n);
    }
    close(pipefd[0]);
    waitpid(pid, NULL, 0);

    return output;
}


std::string CGI_handler::handle_cgi(HttpRequest &request)
{
    std::string cgi_output = "<h1> Not implemented </h1>";
    std::string url = request.getUri();
    printf("Handling CGI for URL: %s\n", url.c_str());

    std::string script_path;
    script_path = request.getPath();

    if (url.size() >= 4 && url.substr(url.size() - 4) == ".php") {
        cgi_output = execute_cgi_script(script_path, "/usr/bin/php-cgi", request);
        std::cout << script_path << std::endl;
        return HttpResponse::create_response(OK, cgi_output);
    }
    if (url.size() >= 3 && url.substr(url.size() - 3) == ".py") {
        cgi_output = execute_cgi_script(script_path, "/usr/bin/python3", request);
        return HttpResponse::create_response(OK, cgi_output);
    }
    return ( HttpResponse::create_response(InternalError, cgi_output) );
}


