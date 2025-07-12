/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   multiplexer.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yimizare <yimizare@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/09 21:46:09 by yimizare          #+#    #+#             */
/*   Updated: 2025/07/11 21:26:57 by yimizare         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "multiplexer.hpp"

Multiplexer* Multiplexer::instance = NULL;

Multiplexer::Multiplexer()
{}

Multiplexer* Multiplexer::getInstance() {
	if (instance == NULL) {
		instance = new Multiplexer();
    }
    return instance;
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
	std::cout << listen_fd << "\n\n";
	if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
		throw std::runtime_error("setsockopt failure");
	sockaddr_in addr;

	std::memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(port);
	
	if (bind(listen_fd, (sockaddr*)&addr, sizeof(addr)) < 0)
	{
		throw std::runtime_error("bind failure");
	}
	if (listen(listen_fd, 10) < 0)
		throw std::runtime_error("Listen failure");
	std::cout << "server up on: localhost:" << port << std::endl;
	return listen_fd;
}

void Multiplexer::modifyEpollEvents(int fd, uint32_t events)
{
    struct epoll_event ev;
    ev.events = events;
    ev.data.fd = fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &ev) < 0) {
        perror("epoll_ctl: mod");
        // Optionally handle error: close fd, erase client state, etc.
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
    ClientState &state = client_states[client_fd];
	HttpResponse response;
	char buf[4096];
    while (true)
	{
        ssize_t n = recv(client_fd, buf, sizeof(buf), 0);
        if (n == 0) {
            close(client_fd);
            client_states.erase(client_fd);
            return;
        }
        if (n < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                break; // No more data for now
            perror("recv error");
            close(client_fd);
            client_states.erase(client_fd);
            return;
        }
        state.buffer.append(buf, n);
    }
    try {
        state.request.parse(state.buffer);
    } catch(const ParseResult& e) {
        if (e != OK && e != Incomplete) {
            state.response_buffer = "HTTP/1.1 400 BadRequest\r\n"
                "Content-Length: 11\r\n"
                "Content-Type: text/plain\r\n"
                "\r\n"
                "NOT FOUND!\n";
            // state.request.setReadStatus(END);
        }
		state.buffer.clear();     
    }
    if (state.request.getReadStatus() == END) {
		if ( state.request.getMethod() == "POST" )
		{
			std::vector<std::string> stored_bodies;
			state.response_buffer = response.handle_post( state.request, stored_bodies);
		}
		else if ( state.request.getMethod() == "GET" )
		{
			response.handle_get(state.request, state.response_buffer);
		}
		state.buffer.clear();
        modifyEpollEvents(client_fd, EPOLLOUT);
    }
}

void Multiplexer::handleClientWrite(int client_fd)
{
    ClientState &state = client_states[client_fd];
    // Make sure there is something to send
    if (state.response_buffer.empty()) {
        // Nothing to send, switch back to EPOLLIN
        modifyEpollEvents(client_fd, EPOLLIN);
        return;
    }

    ssize_t n = send(client_fd, state.response_buffer.c_str(), state.response_buffer.size(), 0);
   if (n < 0)
   {
    if (errno == EAGAIN || errno == EWOULDBLOCK)
	{
        // Socket not ready for writing, try again later
        return;
    }
    	perror("send");
    	close(client_fd);
    	client_states.erase(client_fd);
    	return;
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
            close(client_fd);
            client_states.erase(client_fd);
        }
    }
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
    char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(client_addr.sin_addr), ip, INET_ADDRSTRLEN);
    int client_port = ntohs(client_addr.sin_port);
    std::cout << "\033[36mNEW CONNECTION FROM...  " << ip << ":" << client_port << "\033[0m" << std::endl;
	client_states[client_fd] = ClientState();
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

void 	Multiplexer::run()
{
	epoll_fd = epoll_create1(EPOLL_CLOEXEC);
    if (epoll_fd < 0)
	{
		if (errno == EMFILE || errno == ENFILE)
		{
			std::cerr << "Fatal: File descriptor limit reached (errno " << errno << ")." << std::endl;
		}
		else
			perror("epoll_create1");
        throw std::runtime_error("Failed to create epoll instance");
    }

    // 2. Set up listening sockets (assuming you have a vector of ports)
    std::vector<int> listen_fds;
    std::set<int> listen_fd_set; // For quick lookup in is_listening_socket
    for (size_t i = 0; i < server_ports.size(); ++i) {
        int fd = SetupServerSocket(server_ports[i]);
        listen_fds.push_back(fd);
        listen_fd_set.insert(fd);

        struct epoll_event ev;
        ev.events = EPOLLIN;
        ev.data.fd = fd;
        if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &ev) < 0) {
            perror("epoll_ctl: listen_fd");
            close(fd);
            throw std::runtime_error("Failed to add listen_fd to epoll");
        }
    }

	// main event loop :
	const int MAX_EVENTS = 1024;
    struct epoll_event events[MAX_EVENTS];
	
	while (true)
	{
		int n = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
		if (n < 0)
		{
			if (errno == EINTR){ // signals ?
				continue ;
			}
			perror("epoll_wait");
			break ;
		}
		for (int i = 0; i < n; ++i)
		{
			int fd = events[i].data.fd;
			uint32_t ev = events[i].events;
			if (listen_fd_set.count(fd))
			{
        		handleNewConnection(fd);
    		}
			else
			{
        		if (ev & EPOLLIN) {
        		   handleClientRead(fd); // Read request
        		}
        		if (ev & EPOLLOUT) {
        		    handleClientWrite(fd); // Write response
        		}
        		if (ev & (EPOLLERR | EPOLLHUP))
				{
					perror("epoll error or hangup");
					epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL); // Remove from epoll
    				close(fd);                                    // Close the socket
    				client_states.erase(fd);                      // Remove client state
    				continue ;
					// Handle errors or closed connection
        		}
    		}
		}
	}
	for (size_t i = 0; i < listen_fds.size(); ++i)
		close(listen_fds[i]);
	close(epoll_fd);
}