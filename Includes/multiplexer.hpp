/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   multiplexer.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yimizare <yimizare@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/09 21:46:26 by yimizare          #+#    #+#             */
/*   Updated: 2025/07/08 14:13:46 by yimizare         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef MULTIPLEXER_HPP
# define MULTIPLEXER_HPP

#include "includes.hpp"

class ClientState
{
	public:
		std::string buffer;
		size_t bytes_read;
		bool request_complete;
		std::string response_buffer;
    	int cgi_pid;
    	std::string upload_path;
};

class Multiplexer
{
	private:
		Multiplexer();
		int	listen_fd;
		int	epoll_fd;
		static Multiplexer* instance;
		int SetupServerSocket(int port);
		void handleNewConnection(int listen_fd);
		void handleClient(int client_fd);
		std::map<int, ClientState> client_states;
	public :
		std::vector<int> server_ports;
		static Multiplexer* getInstance();
		void run();
};




#endif