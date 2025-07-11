#include <iostream>
#include <sys/socket.h> // socket
#include <netinet/in.h> // for socketaddr_in
#include <unistd.h>
#include <cerrno>
#include <cstring>
#include "HttpRequest.hpp"

#define BUFFER_SIZE 145445

using namespace std;



void requestTester(HttpRequest &http) 
{
    std::string requests[10] = {
        // 1. Valid GET request
        "GET /index.html HTTP/1.1\r\nHost: www.example.com\r\n\r\n",

        // 2. Valid POST request with headers and body
        "POST /submit-form HTTP/1.1\r\nHost: www.example.com\r\nContent-Length: 11\r\n\r\nHello=World",

        // 3. Invalid request: Missing HTTP version
        "GET /home\r\nHost: example.com\r\n\r\n",

        // 4. Invalid request: Malformed request line (missing method)
        "/index.html HTTP/1.1\r\nHost: www.example.com\r\n\r\n",

        // 5. Invalid request: Unsupported HTTP method
        "FETCH /data HTTP/1.1\r\nHost: example.com\r\n\r\n",

        // 6. Valid HEAD request
        "HEAD / HTTP/1.0\r\nHost: localhost\r\n\r\n",

        // 7. Invalid request: Headers not properly separated by CRLF
        "GET / HTTP/1.1\nHost: example.com\n\n",

        // 8. Valid request with query string
        "GET /search?q=openai HTTP/1.1\r\nHost: www.example.com\r\n\r\n",

        // 9. Invalid request: Missing CRLF between headers and body
        "POST /api HTTP/1.1\r\nHost: example.com\r\nContent-Length: 5\r\nHello",

        // 10. Valid PUT request with JSON body
        "PUT /api/item/123 HTTP/1.1\r\nHost: api.example.com\r\nContent-Type: application/json\r\nContent-Length: 18\r\n\r\n{\"name\":\"test\"}"
    };

    for (int i = 0; i < 10; i++) {
        cout << "-------------------[" << i + 1 << "] -----------------" << endl;
        if (http.parse(requests[i]) == BadRequest) {
            cout << "HTTP/1.1 400 Bad Request\r\n\r\n" << endl;
        }
    }         
}


int server()
{
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        cerr << "Error: socket failed." << endl;
        return (1);
    }
    sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(8080); // fix the little endian and big endian problem
    server_address.sin_addr.s_addr = INADDR_ANY;

    int opt = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        cerr << "Error: setsockopt" << endl;
        return (1);
    }
    if (bind(server_socket, (struct sockaddr*)&server_address, sizeof(server_address)) == -1) {
        cerr << "Error: bind failed." << endl;
        return (1);
    }

    if (listen(server_socket,  SOMAXCONN))
    {
        cerr << "Error:Server socket failed to listen." << endl;
        return (1);
    }
    cout << " Server is now listening on port [" << ntohs(server_address.sin_port) << "]" << endl;
    int client_socket = accept(server_socket, NULL, NULL);
    if (client_socket == -1)
    {
        cerr << "Error: Server cannot accept connection with a client\n" << endl;
        return (1);
    }
    
    string request;
    HttpRequest http;
    const char* response = NULL;

    while (http.getReadStatus() != END) {
        char buffer[BUFFER_SIZE] = {0};
        int byte_read = recv(client_socket, buffer, BUFFER_SIZE, 0);
        if (byte_read == 0) {
            break ;
        }
        if (byte_read == -1) {
            cerr << "Error: Failed to receive message from the client." << endl;
            return (1);
        }
        try
        {
            // std::cout << "start parse" << std::endl;
            std::string request(buffer, byte_read);
            http.parse(request) ;
            // std::cout << "end parse" << std::endl;

        }
        catch(const ParseResult& e)
        {
            if (e != OK && e != Incomplete)
            {
                response = "HTTP/1.1 400 BadRequest\r\n"
                        "Content-Length: 11\r\n"
                        "Content-Type: text/plain\r\n"
                        "\r\n"
                        "NOT FOUND!\n";
                http.setReadStatus( END );
                send(client_socket, response, strlen(response), 0);    
                close(client_socket);
                close(server_socket);
                return (1);
            }
        }
    }
    std::cout << "Received body:\n" << http.getBody() << std::endl;
    std::cout << "keep alive= " << http.getKeepAlive() << std::endl;
    std::cout << "content_type=" << http.getContentType() << std::endl;    
    std::cout << "Server Is [" << http.getServer().server_name << "]\n" << std::endl;
    std::cout << "Location Is [" << http.getLocation().path << "]\n" << std::endl;
    std::cout << "Path Is [" << http.getPath() << "]\n" << std::endl;
    response = "HTTP/1.1 200 OK\r\n"
    "Content-Length: 13\r\n"
    "Content-Type: text/plain\r\n"
    "\r\n"
    "Hello, world!";
    
    send(client_socket, response, strlen(response), 0);    
    close(client_socket);
    close(server_socket);
    return (0);
}