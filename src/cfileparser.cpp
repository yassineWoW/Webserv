/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cfileparser.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yimizare <yimizare@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/02 18:13:25 by yimizare          #+#    #+#             */
/*   Updated: 2025/07/03 20:44:01 by yimizare         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "cfileparser.hpp"


ConfigParser *ConfigParser::instance = NULL;

ConfigParser::ConfigParser(const std::string &filename)
{
	std::ifstream file(filename.c_str());
	if (!file) {
    std::cerr << "Could not open config file: " << filename << std::endl;
    	return;
		}
	std::stringstream buffer;
	buffer << file.rdbuf();
	
	std::string content = buffer.str();
	std::vector<std::string> tokens = ConfigParser::tokenize(content);
	servers = ConfigParser::parse(tokens);
}

ConfigParser *ConfigParser::getInstance(const std::string& filename)
{
	if (!instance)
	{
		instance = new ConfigParser(filename);
	}
	return instance;
}

std::vector<ServerConfig> &ConfigParser::getServers()
{
	return servers;
}

std::vector<std::string> ConfigParser::tokenize(const std::string &input)
{
	std::vector<std::string> tokens;
	std::string token;
	
	for (size_t i = 0; i < input.size(); i++)
	{
		char c = input[i];
		if (std::isspace(c))
		{
			if (!token.empty())
			{
				tokens.push_back(token);
				token.clear();
			}
		}
		else if (c == '{' || c == '}' || c == ';')
		{
			if (!token.empty())
			{
				tokens.push_back(token);
				token.clear();
			}
			tokens.push_back(std::string(1, c));
		}
		else
		{
			token += c;
		}
	}
	if (!token.empty())
	{
		tokens.push_back(token);
	}
	return tokens;
}


size_t parse_location(const std::vector<std::string> &tokens, size_t i, LocationConfig &location)
{
	location.path = tokens[i + 1];
	i += 3;
	while (tokens[i] != "}")
	{
		if (tokens[i] == "root")
		{
			location.root = tokens[i + 1];
			i += 3;
		}
		else if (tokens[i] == "index")
		{
			location.index = tokens[i + 1];
			i += 3;
		}
		else
		{
			i++;
		}
	}
	return (i + 1);
}


std::vector<ServerConfig> ConfigParser::parse(const std::vector<std::string> tokens)
{
	std::vector<ServerConfig> servers;
	size_t i = 0;
	
	while (i < tokens.size())
	{
		if (tokens[i] == "server" && tokens[i + 1] == "{")
		{
			ServerConfig server;
			i += 2;
			while (tokens[i] != "}")
			{
				if (tokens[i] == "listen")
				{
					server.listen_port = atoi(tokens[i + 1].c_str());
					i += 3;
				}
				else if (tokens[i] == "server_name")
				{
					server.server_name = tokens[i + 1];
					i += 3;
				}
				else if (tokens[i] == "location")
				{
					LocationConfig location;
					i = parse_location(tokens, i, location);
					server.locations.push_back(location);
				}
				else
					i++;
			}
			servers.push_back(server);
		}
		++i;
	}
	return (servers);
}

//void mainparser()
//{
//	std::ifstream file("webserv.conf");
//	std::stringstream buffer;
//	buffer << file.rdbuf();
	
//	std::string content = buffer.str();
//	std::vector<std::string> tokens = ConfigParser::tokenize(content);
//	std::vector<ServerConfig> servers = ConfigParser::parse(tokens);
//	//for (size_t i = 0; i < tokens.size(); i++)
//	//	std::cout << "[" << tokens[i] << "]" << std::endl;
	
//	std::cout << "Tokens:\n";
////for (size_t i = 0; i < tokens.size(); i++)
////    std::cout << "[" << tokens[i] << "]" << std::endl;

////std::cout << "Servers parsed: " << servers.size() << std::endl;

//	std::cout << servers.size() << std::endl;
//	for (size_t i = 0; i < servers.size(); ++i) 
//	{
//        std::cout << "Server " << i << ":\n";
//        std::cout << "  Port: " << servers[i].listen_port << "\n";
//        std::cout << "  Server Name: " << servers[i].server_name << "\n";
//        for (size_t j = 0; j < servers[i].locations.size(); ++j) {
//            std::cout << "    Location " << servers[i].locations[j].path << ":\n";
//            std::cout << "      Root: " << servers[i].locations[j].root << "\n";
//            std::cout << "      Index: " << servers[i].locations[j].index << "\n";
//        }
//    }
//}