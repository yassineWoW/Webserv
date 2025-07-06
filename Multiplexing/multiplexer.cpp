/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   multiplexer.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yimizare <yimizare@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/09 21:46:09 by yimizare          #+#    #+#             */
/*   Updated: 2025/07/01 21:39:29 by yimizare         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "multiplexer.hpp"

Multiplexer* Multiplexer::instance = NULL;


Multiplexer* Multiplexer::getInstance(int port) {
	if (instance == NULL) {
		instance = new Multiplexer(port);
    }
    return instance;
}

int Multiplexer::SetupServerSocket(int port)
{
	listen_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_fd < 0)
	{
		throw std::runtime_error("Socket creation failure\n");
	}
	int opt = 1;
	if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt) < 0))
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
	return listen_fd;
}

Multiplexer::Multiplexer(int port) {
	SetupServerSocket(port);
}