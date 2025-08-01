/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   multiplexer.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yimizare <yimizare@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/09 21:46:26 by yimizare          #+#    #+#             */
/*   Updated: 2025/07/30 13:07:45 by yimizare         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef MULTIPLEXER_HPP
# define MULTIPLEXER_HPP

#include "includes.hpp"
#include "HttpResponse.hpp"

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
		int SetupServerSocket(int port);
		void handleNewConnection(int listen_fd);
		void handleClientRead(int client_fd);
		void handleClientWrite(int client_fd);
		std::map<int, ClientState> client_states;
		public :
		~Multiplexer();
			static void desroyInstance();
			std::vector<int> server_ports;
			static Multiplexer* getInstance();
			void modifyEpollEvents(int fd, uint32_t events);
			void run();
};




#endif