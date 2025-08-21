/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   multiplexer.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yimizare <yimizare@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/09 21:46:26 by yimizare          #+#    #+#             */
/*   Updated: 2025/08/21 15:22:46 by yimizare         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef MULTIPLEXER_HPP
# define MULTIPLEXER_HPP

#include "includes.hpp"
#include "HttpResponse.hpp"
#include "Cgi.hpp"


class Cgi;
class ClientState
{
	public:
		std::string buffer;
		size_t bytes_read;
		bool request_complete;
		std::string response_buffer;
    	int cgi_pid;
    	std::string upload_path;
		bool keep_alive;
		HttpRequest request;
		time_t last_activity;
		bool is_cgi_running;
		time_t cgi_start_time;
};

class Multiplexer
{
	private:
		Multiplexer();
		int	listen_fd;
		std::vector<int> listen_fds;
		std::set<int> listen_fd_set;
		int	epoll_fd;
		static Multiplexer* instance;
		std::map<int, int> cgi_fd_to_client_fd;
		std::map<int, pid_t> cgi_fd_to_pid;  // Track CGI process IDs
		int SetupServerSocket(int port);
		void handleNewConnection(int listen_fd);
		void handleClientRead(int client_fd);
		void handleClientWrite(int client_fd);
		void handleCgiRead(int cgi_fd);
		void cleanupCgiProcess(int cgi_fd);  // New method for CGI cleanup
		void checkCgiTimeouts();  // New method for CGI timeout checking
		std::map<int, ClientState> client_states;
	public :
		~Multiplexer();
			static void desroyInstance();
			std::vector<int> server_ports;
			static Multiplexer* getInstance();
			void modifyEpollEvents(int fd, uint32_t events);
			void run();
			void registerCgiFd(int cgi_fd, int client_fd);
};
const char* my_inet_ntop(int af, const void* src, char* dst, socklen_t size);



#endif