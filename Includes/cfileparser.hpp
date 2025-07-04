/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cfileparser.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yimizare <yimizare@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/02 18:13:39 by yimizare          #+#    #+#             */
/*   Updated: 2025/07/03 20:14:02 by yimizare         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CFILEPARSER_HPP
# define CFILEPARSER_HPP

#include "includes.hpp"

struct LocationConfig
{
	std::string path;
	std::string root;
	std::string index;
	bool autoindex;
	std::vector<std::string> allowed_methods;
	std::string redirection;
	std::string cgi_pass; 
	std::string upload_path;
};

struct ServerConfig
{
	int listen_port;
	std::string server_name;
	std::vector<LocationConfig> locations;
	std::map<int, std::string> error_pages;
};





class ConfigParser
{
	private :
		std::string fileContent;
		static ConfigParser *instance;
		std::vector<ServerConfig> servers;
		ConfigParser(const std::string &filename);
	public :
		static std::vector<std::string> tokenize(const std::string &input);
		static std::vector<ServerConfig> parse(const std::vector<std::string> tokens);
		static ConfigParser *getInstance(const std::string& filename);
		std::vector<ServerConfig> &getServers();
};

void mainparser();
size_t parse_location(const std::vector<std::string> &tokens, size_t i, LocationConfig &location);

#endif