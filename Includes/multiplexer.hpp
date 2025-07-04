/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   multiplexer.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yimizare <yimizare@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/09 21:46:26 by yimizare          #+#    #+#             */
/*   Updated: 2025/07/01 21:40:46 by yimizare         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef MULTIPLEXER_HPP
# define MULTIPLEXER_HPP

#include "includes.hpp"

class Multiplexer
{
	private:
		Multiplexer(int port);
		int	listen_fd;
		int	epoll_fd;
		static Multiplexer* instance;
		int SetupServerSocket(int port);
	/*TO DO :
	{
		void handleNewConnection();
		void handleClient(int client_fd);
		}
		*/
	public :
		static Multiplexer* getInstance(int port);
		void run();
};




#endif