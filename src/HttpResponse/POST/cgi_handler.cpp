#include "Cgi.hpp"


CGI_handler::CGI_handler(): r_buffer(""), r_current_body_size(0) { }


std::string CGI_handler::execute_cgi_script(const std::string& script_path, const std::string& interpreter) {

    // std::cout << script_path << " or " << interpreter << " is empty\n";

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
        // Child process
        dup2(pipefd[1], STDOUT_FILENO);
        dup2(pipefd[1], STDERR_FILENO);
        close(pipefd[0]);
        close(pipefd[1]);
        char* const argv[] = { (char*)interpreter.c_str(), (char*)script_path.c_str(), NULL };
        execv(interpreter.c_str(), argv);
        perror("execv failed");
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
    // printf("CGI script executed: %s\n", output.c_str());
    return output;
}


std::string CGI_handler::handle_cgi(HttpRequest &request)
{
    std::string cgi_output = "<h1> Not implemented </h1>";
    std::string url = request.getUri();
    printf("Handling CGI for URL: %s\n", url.c_str());

    // Construct the correct script path
    std::string script_path;
    // if (!request.getLocation().root.empty()) {
    //     // Remove leading '/' from URI if present
    //     std::string uri = url;
    //     if (!uri.empty() && uri[0] == '/')
    //         uri = uri.substr(1);
    //     printf("URI after removing leading '/': %s\n", request.getPath().c_str());
    //     script_path = request.getLocation().root + "/" + uri;
    //     // printf("Script path: %s\n", script_path.c_str());
    // } else {
        script_path = request.getPath();
    // }

    if (url.size() >= 4 && url.substr(url.size() - 4) == ".php") {
        cgi_output = execute_cgi_script(script_path, "/usr/bin/php-cgi");
        std::cout << script_path << std::endl;
        return HttpResponse::create_response(OK, cgi_output);
    }
    if (url.size() >= 3 && url.substr(url.size() - 3) == ".py") {
        cgi_output = execute_cgi_script(script_path, "/usr/bin/python3");
        return HttpResponse::create_response(OK, cgi_output);
    }
    return ( HttpResponse::create_response(InternalError, cgi_output) );
}