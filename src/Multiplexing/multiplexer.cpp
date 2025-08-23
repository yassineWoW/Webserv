/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   multiplexer.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yimizare <yimizare@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/09 21:46:09 by yimizare          #+#    #+#             */
/*   Updated: 2025/08/10 16:50:54 by yimizare         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "multiplexer.hpp"

extern volatile sig_atomic_t stop_server;

Multiplexer* Multiplexer::instance = NULL;

Multiplexer::Multiplexer()
{}

void Multiplexer::desroyInstance()
{
	if (instance)
	{
		delete instance;
		instance  = NULL;
	}
}

Multiplexer* Multiplexer::getInstance() {
	if (instance == NULL) {
		instance = new Multiplexer();
    }
    return instance;
}

Multiplexer::~Multiplexer()
{
    // Close all listening sockets
    for (size_t i = 0; i < listen_fds.size(); ++i)
        close(listen_fds[i]);
    // Close all client sockets
    for (std::map<int, ClientState>::iterator it = client_states.begin(); it != client_states.end(); ++it)
        close(it->first);
    client_states.clear();
    // Close epoll fd
    if (epoll_fd >= 0)
        close(epoll_fd);
}

int Multiplexer::SetupServerSocket(int port)
{
	listen_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_fd < 0)
	{
		if(errno == EMFILE || errno == ENFILE)
		{
			std::cerr << "Fatal: File descriptor limit reached during socket() (errno " << errno << ")." << std::endl;
		}
		else
			perror("socket");
		throw std::runtime_error("Socket creation failure\n");
	}
	int opt = 1;
	// std::cout << listen_fd << "\n\n";
	if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
	{
		close(listen_fd);
		throw std::runtime_error("setsockopt failure");
	}
	sockaddr_in addr;

	std::memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(port);
	
	if (bind(listen_fd, (sockaddr*)&addr, sizeof(addr)) < 0)
	{
		close(listen_fd);
		throw std::runtime_error("bind failure");
	}
	if (listen(listen_fd, 10) < 0)
	{
		close(listen_fd);
		throw std::runtime_error("Listen failure");
	}
	std::cout << "server up on: localhost:" << port << std::endl;
	return listen_fd;
}

void Multiplexer::modifyEpollEvents(int fd, uint32_t events)
{
    // Check if the file descriptor is valid
    if (fd < 0) {
        return;
    }
    
    // Check if client still exists (for client fds)
    if (client_states.find(fd) == client_states.end() && 
        listen_fd_set.find(fd) == listen_fd_set.end() &&
        cgi_fd_to_client_fd.find(fd) == cgi_fd_to_client_fd.end()) {
        // FD not found in any of our tracking structures
        return;
    }
    
    struct epoll_event ev;
    ev.events = events;
    ev.data.fd = fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &ev) < 0) {
        if (errno == ENOENT) {
            // FD not in epoll, this is expected in some cleanup scenarios
            return;
        } else if (errno == EBADF) {
            // Bad file descriptor - fd was closed
            std::cerr << "Warning: Attempted to modify epoll events for closed fd " << fd << std::endl;
            return;
        } else {
            perror("epoll_ctl: mod");
        }
    }
}

//void Multiplexer::handleClientRead(int client_fd) 
//{
//	char buf[4096];
//	ssize_t n = recv(client_fd, buf, sizeof(buf), 0);
//	if (n == 0)
//	{
//		close(client_fd);
//		client_states.erase(client_fd);
//		return ;
//	}
//	if (n < 0)
//	{
//		if (errno == EAGAIN || errno == EWOULDBLOCK)
//			return ;
//		perror("recv error");
//		close(client_fd);
//    	client_states.erase(client_fd);
//		return ;
//	}
//	ClientState &state = client_states[client_fd];
//	state.buffer.append(buf,n);
//	try
//	{
//		state.request.parse(state.buffer);
//	}
//	catch(const ParseResult& e)
//	{
//		if (e != OK && e != Incomplete)
//           {
//                state.response_buffer = "HTTP/1.1 400 BadRequest\r\n"
//                        "Content-Length: 11\r\n"
//                        "Content-Type: text/plain\r\n"
//                        "\r\n"
//                        "NOT FOUND!\n";
//                state.request.setReadStatus( END );       
//           }
//	}
	
//	if (state.request.getReadStatus() == END)
//	{
//	    state.response_buffer = "HTTP/1.1 200 OK\r\n"
//	    "Content-Length: 13\r\n"
//	    "Content-Type: text/plain\r\n"
//	    "\r\n"
//	    "Hello, world!";
//		std::cout << "Received body:\n" << state.request.getBody() << std::endl;
//		modifyEpollEvents(client_fd, EPOLLOUT);
//	}
//}

void Multiplexer::handleClientRead(int client_fd) 
{
    // Validate client_fd first
    if (client_states.find(client_fd) == client_states.end()) {
        // Client no longer exists, skip processing
        return;
    }
    
	ClientState &state = client_states[client_fd];
    state.last_activity = time(NULL);
    HttpResponse response;
    char buf[4096];

    ssize_t n = recv(client_fd, buf, sizeof(buf), 0);
    if (n == 0) {
        // Client disconnected
        epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_fd, NULL);
        close(client_fd);
        client_states.erase(client_fd);
        return;
    }
    if (n < 0)
	{
        return;
    }
    state.buffer.append(buf, n);
    try 
    {
        state.request.parse(state.buffer);
        if (state.request.getReadStatus() == END) 
        {
            ParseResult server_result = match_server_location( state.request,  state.response_buffer);
            std::string url = state.request.getUri();
            int f_cgi = 0;
            if ( (url.size() >= 4 && url.substr(url.size() - 4) == ".php") || (url.size() >= 3 && url.substr(url.size() - 3) == ".py"))
                f_cgi = 1;
            if ( server_result != OK)
                throw ( server_result );
            else if ( state.request.getMethod() == "POST" )
            {
                std::vector<std::string> stored_bodies;
                if (f_cgi)
                {
                    CGI_handler cgi;
                    std::string cgi_result = cgi.handle_cgi(state.request, true);
                    if (cgi_result.substr(0, 12) == "CGI_STARTED:")
                    {
                        std::string remaining = cgi_result.substr(12);
                        size_t colon1 = remaining.find(':');
                        size_t colon2 = remaining.find(':', colon1 + 1);
                        if (colon1 != std::string::npos && colon2 != std::string::npos)
                        {
                            int cgi_fd = atoi(remaining.substr(0, colon1).c_str());
                            pid_t cgi_pid = atoi(remaining.substr(colon1 + 1, colon2 - colon1 - 1).c_str());
                            int stdin_fd = atoi(remaining.substr(colon2 + 1).c_str());
                            cgi_fd_to_pid[cgi_fd] = cgi_pid;
                            registerCgiFd(cgi_fd, client_fd);
                            const std::string& body = state.request.getBody();
                            ssize_t written = 0;
                            size_t total = 0;
                            while (total < body.size())
                            {
                                written = write(stdin_fd, body.data() + total, body.size() - total);
                                if (written <= 0) break;
                                total += written;
                            }
                            close(stdin_fd);
                            return;
                        }
                    } else
                        state.response_buffer = cgi_result;
                }
                else
                {
                    // if (state.request.getCookies().find("PHPSESSID") == state.request.getCookies().end())
                    //     state.response_buffer = handle_redirection( "302", "/login");
                    // else
                        state.response_buffer = response.handle_post( state.request, stored_bodies);
                }
            }
            else if (state.request.getMethod() == "GET")
            {
                std::string url = state.request.getUri();
                if ( url == "/login/Cookies.php" && state.request.getCookies().find("PHPSESSID") == state.request.getCookies().end())
                        state.response_buffer = handle_redirection( "302", "/login");
                else if (f_cgi)
                {
                    std::cout << "---------------------------" << state.request.getPath() << std::endl;
                    ParseResult isOk = isFileAndAccessible(state.request.getPath(), W_OK);
                    if (isOk != OK)
                    {
                        throw isOk;
                    }
                    CGI_handler cgi;
                    std::string cgi_result = cgi.handle_cgi( state.request, false );
                    
                    if (cgi_result.substr(0, 12) == "CGI_STARTED:")
                    {
                        std::string remaining = cgi_result.substr(12);
                        size_t colon_pos = remaining.find(':');
                        if (colon_pos != std::string::npos)
                        {
                            int cgi_fd = atoi(remaining.substr(0, colon_pos).c_str());
                            pid_t cgi_pid = atoi(remaining.substr(colon_pos + 1).c_str());
                            cgi_fd_to_pid[cgi_fd] = cgi_pid;
                            registerCgiFd(cgi_fd, client_fd);
                            return;
                        }
                    } else
                        state.response_buffer = cgi_result;
                }
                else
                    response.handle_get(state.request, state.response_buffer);
            }
            else if ( state.request.getMethod() == "DELETE" )
                response.handle_delete(state.request, state.response_buffer);
            state.buffer.clear();
            if (!state.is_cgi_running)
                modifyEpollEvents(client_fd, EPOLLOUT);
        }
    
    }
    catch(const ParseResult& e) 
    {
        if (e != OK && e != Incomplete)
        {
            Errors error;
            state.response_buffer = error.handle_error( state.request.getServer().error_pages, e ) ;
            modifyEpollEvents(client_fd, EPOLLOUT);
        }
        state.buffer.clear();     
    }
}

void Multiplexer::handleClientWrite(int client_fd)
{
    // Validate client_fd first
    if (client_states.find(client_fd) == client_states.end()) {
        // Client no longer exists, skip processing
        return;
    }
    
    ClientState &state = client_states[client_fd];
	state.last_activity = time(NULL);
    // Make sure there is something to send
    if (state.response_buffer.empty()) {
        // Nothing to send, switch back to EPOLLIN
        modifyEpollEvents(client_fd, EPOLLIN);
        return;
    }

    ssize_t n = send(client_fd, state.response_buffer.c_str(), state.response_buffer.size(), 0);
   if (n < 0)
   {
		return ;
   }

    // Remove sent data from the buffer
    state.response_buffer.erase(0, n);

    if (state.response_buffer.empty())
	{
        // All data sent!
        if (state.keep_alive)
		{
            // If keep-alive, switch back to EPOLLIN for next request
            state.request_complete = false;
            state.buffer.clear();
            modifyEpollEvents(client_fd, EPOLLIN);
        }
		else
		{
            // Otherwise, close connection
            epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_fd, NULL);
            close(client_fd);
            client_states.erase(client_fd);
        }
    }
}

const char* my_inet_ntop(int af, const void* src, char* dst, socklen_t size)
{
    if (af != AF_INET || !src || !dst || size < 16) {
        return NULL; // Invalid parameters
    }
    
    // Cast the source to unsigned char pointer to access individual bytes
    const unsigned char* addr = (const unsigned char*)src;
    
    // Extract each byte of the IP address
    // IPv4 addresses are stored in network byte order (big-endian)
    unsigned char byte1 = addr[0];  // First octet
    unsigned char byte2 = addr[1];  // Second octet
    unsigned char byte3 = addr[2];  // Third octet
    unsigned char byte4 = addr[3];  // Fourth octet
    
    // Convert each byte to string and format as "xxx.xxx.xxx.xxx"
    int written = snprintf(dst, size, "%u.%u.%u.%u", 
                          (unsigned int)byte1,
                          (unsigned int)byte2, 
                          (unsigned int)byte3,
                          (unsigned int)byte4);
    
    // Check if the conversion was successful
    if (written < 0 || written >= (int)size) {
        return NULL; // Buffer too small or error
    }
    
    return dst; // Return pointer to the result string
}


void	Multiplexer::handleNewConnection(int listen_fd)
{
	sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    int client_fd = accept(listen_fd, (sockaddr*)&client_addr, &client_len);
    if (client_fd < 0)
    {
		if (errno == EMFILE || errno == ENFILE)
		{
			std::cerr << "Fatal: File descriptor limit reached during accept() (errno " << errno << ")." << std::endl;
		}
		else
		perror("accept");
		throw std::runtime_error("Accept failure"); // or throw an exception
    }
    char ip[16];
	my_inet_ntop(AF_INET, &(client_addr.sin_addr), ip, sizeof(ip));
    int client_port = ntohs(client_addr.sin_port);
    std::cout << "\033[36mNEW CONNECTION FROM...  " << ip << ":" << client_port << "\033[0m" << std::endl;
	client_states[client_fd] = ClientState();
	client_states[client_fd].last_activity = time(NULL);
	client_states[client_fd].is_cgi_running = false;
	int flags = fcntl(client_fd, F_GETFL, 0);
	if (flags < 0 || fcntl(client_fd, F_SETFL, flags | O_NONBLOCK) < 0)
    {
		perror("fcntl");
        close(client_fd);
        client_states.erase(client_fd);
        return;
    }
	struct epoll_event ev;
	ev.events = EPOLLIN;
	ev.data.fd = client_fd;
	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &ev) < 0)
	{
		perror("epoll_ctl: client_fd");
		close(client_fd);
		client_states.erase(client_fd);
		return ;
	}
}

void Multiplexer::registerCgiFd(int cgi_fd, int client_fd)
{
    cgi_fd_to_client_fd[cgi_fd] = client_fd;
    client_states[client_fd].is_cgi_running = true;
    client_states[client_fd].cgi_start_time = time(NULL);
    
    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = cgi_fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, cgi_fd, &ev) < 0) {
        perror("epoll_ctl: add CGI fd");
        close(cgi_fd);
        cgi_fd_to_client_fd.erase(cgi_fd);
        client_states[client_fd].is_cgi_running = false;
    }
}

void Multiplexer::handleCgiRead(int cgi_fd)
{
    std::map<int, int>::iterator it = cgi_fd_to_client_fd.find(cgi_fd);
    if (it == cgi_fd_to_client_fd.end()) {
        // CGI fd not found, cleanup
        epoll_ctl(epoll_fd, EPOLL_CTL_DEL, cgi_fd, NULL);
        close(cgi_fd);
        return;
    }
    
    int client_fd = it->second;
    ClientState &state = client_states[client_fd];
    
    char buffer[4096];
    std::string cgi_output;
    
    // Read all available data from CGI
    ssize_t n;
    while ((n = read(cgi_fd, buffer, sizeof(buffer))) > 0) {
        cgi_output.append(buffer, n);
    }
    // CGI finished - cleanup and prepare response
    cleanupCgiProcess(cgi_fd);
    
    std::string status_line = "HTTP/1.1 200 OK\r\n";
    std::string headers;
    std::string body;

    std::string::size_type header_end = cgi_output.find("\r\n\r\n");
    if (header_end != std::string::npos)
    {
        headers = cgi_output.substr(0, header_end);
        body = cgi_output.substr(header_end + 4);

        std::string::size_type pos = headers.find("Status:");
        if (pos != std::string::npos)
        {
            std::string status_value = headers.substr(pos + 7);
            std::string::size_type end = status_value.find("\r\n");
            if (end != std::string::npos)
                status_value = status_value.substr(0, end);
            
            while (!status_value.empty() && (status_value[0] == ' ' || status_value[0] == '\t'))
                status_value.erase(0, 1);

            status_line = "HTTP/1.1 " + status_value + "\r\n";

            headers.erase(pos, end + 2);
        }
        else if (headers.find("Location:") != std::string::npos)
            status_line = "HTTP/1.1 302 Found\r\n";
    }
    state.response_buffer = status_line + headers + "\r\n\r\n" + body;

    modifyEpollEvents(client_fd, EPOLLOUT);
}

void Multiplexer::cleanupCgiProcess(int cgi_fd)
{
    std::map<int, int>::iterator it = cgi_fd_to_client_fd.find(cgi_fd);
    if (it != cgi_fd_to_client_fd.end()) {
        int client_fd = it->second;
        client_states[client_fd].is_cgi_running = false;
        cgi_fd_to_client_fd.erase(cgi_fd);
    }
    
    // Clean up PID mapping if it exists
    std::map<int, pid_t>::iterator pid_it = cgi_fd_to_pid.find(cgi_fd);
    if (pid_it != cgi_fd_to_pid.end()) {
        cgi_fd_to_pid.erase(pid_it);
    }
    
    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, cgi_fd, NULL);
    close(cgi_fd);
}

void Multiplexer::checkCgiTimeouts()
{
    const int CGI_TIMEOUT = 30; // 30 seconds timeout for CGI
    time_t now = time(NULL);
    
    std::vector<int> timed_out_cgis;
    
    for (std::map<int, int>::iterator it = cgi_fd_to_client_fd.begin(); 
         it != cgi_fd_to_client_fd.end(); ++it) {
        int cgi_fd = it->first;
        int client_fd = it->second;
        
        if (client_states.find(client_fd) != client_states.end()) {
            ClientState &state = client_states[client_fd];
            if (now - state.cgi_start_time > CGI_TIMEOUT) {
                timed_out_cgis.push_back(cgi_fd);
            }
        }
    }
    
    // Kill timed out CGI processes
    for (size_t i = 0; i < timed_out_cgis.size(); ++i) {
        int cgi_fd = timed_out_cgis[i];
        std::map<int, pid_t>::iterator pid_it = cgi_fd_to_pid.find(cgi_fd);
        if (pid_it != cgi_fd_to_pid.end()) {
            std::cout << "Killing timed out CGI process " << pid_it->second << std::endl;
            kill(pid_it->second, SIGKILL);
        }
        
        // Send timeout response to client
        std::map<int, int>::iterator client_it = cgi_fd_to_client_fd.find(cgi_fd);
        if (client_it != cgi_fd_to_client_fd.end()) {
            int client_fd = client_it->second;
            ClientState &state = client_states[client_fd];
            state.response_buffer = HttpResponse::create_response(InternalError, "CGI script timed out");
            modifyEpollEvents(client_fd, EPOLLOUT);
        }
        
        cleanupCgiProcess(cgi_fd);
    }
}

void Multiplexer::run()
{
    epoll_fd = epoll_create1(EPOLL_CLOEXEC);
    if (epoll_fd < 0)
    {
        if (errno == EMFILE || errno == ENFILE)
            std::cerr << "Fatal: File descriptor limit reached (errno " << errno << ")." << std::endl;
        else
            perror("epoll_create1");
        throw std::runtime_error("Failed to create epoll instance");
    }
    for (size_t i = 0; i < server_ports.size(); ++i)
    {
        int fd = SetupServerSocket(server_ports[i]);
        listen_fds.push_back(fd);
        listen_fd_set.insert(fd);

        struct epoll_event ev;
        ev.events = EPOLLIN;
        ev.data.fd = fd;
        if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &ev) < 0)
		{
            perror("epoll_ctl: listen_fd");
            close(fd);
            throw std::runtime_error("Failed to add listen_fd to epoll");
        }
    }

    const int MAX_EVENTS = 1024;
    struct epoll_event events[MAX_EVENTS];
    const int EPOLL_TIMEOUT_MS = 1000; // 1 second
    const int CLIENT_TIMEOUT = 60;     // 60 seconds

    while (!stop_server)
    {
        int n = epoll_wait(epoll_fd, events, MAX_EVENTS, EPOLL_TIMEOUT_MS);
        if (n < 0)
        {
            if (errno == EINTR)
                continue;
            perror("epoll_wait");
            break;
        }
        if (n > 0) {
            for (int i = 0; i < n; ++i)
            {
                int fd = events[i].data.fd;
                uint32_t ev = events[i].events;
                
                if (listen_fd_set.count(fd))
                {
                    handleNewConnection(fd);
                }
                else if (cgi_fd_to_client_fd.count(fd))
                {
                    // This is a CGI fd
                    if (ev & EPOLLIN)
                        handleCgiRead(fd);
                    if (ev & (EPOLLERR | EPOLLHUP))
                    {
                        // Handle CGI pipe errors/hangups
                        std::cout << "CGI fd " << fd << " error or hangup" << std::endl;
                        cleanupCgiProcess(fd);
                    }
                }
                else
                {
                    // This is a client fd
                    // Check for errors first to avoid processing invalid fds
                    if (ev & (EPOLLERR | EPOLLHUP))
                    {
                        std::cout << "Client fd " << fd << " error or hangup" << std::endl;
                        epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL);
                        close(fd);
                        client_states.erase(fd);
                        continue;
                    }
                    
                    // Only process read/write if fd is still valid
                    if (client_states.find(fd) != client_states.end()) {
                        if (ev & EPOLLIN)
                            handleClientRead(fd);
                        if (ev & EPOLLOUT)
                            handleClientWrite(fd);
                    }
                }
            }
        }
        // Timeout check runs every second
        time_t now = time(NULL);
        
        // Check for CGI timeouts
        checkCgiTimeouts();
        
        for (std::map<int, ClientState>::iterator it = client_states.begin(); it != client_states.end(); )
        {
			// Don't timeout clients that are waiting for CGI (CGI has its own timeout)
			if (!it->second.is_cgi_running && now - it->second.last_activity > CLIENT_TIMEOUT)
            {
                epoll_ctl(epoll_fd, EPOLL_CTL_DEL, it->first, NULL);
                close(it->first);
                client_states.erase(it++);
            }
            else
            {
                ++it;
            }
        }
    }
    for (size_t i = 0; i < listen_fds.size(); ++i)
        close(listen_fds[i]);
    for (std::map<int, ClientState>::iterator it = client_states.begin(); it != client_states.end(); ++it)
        close(it->first);
    client_states.clear();
    if (epoll_fd >= 0)
        close(epoll_fd);
}

// No changes needed: epoll event loop already supports multiple clients.