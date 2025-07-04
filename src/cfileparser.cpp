/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cfileparser.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yimizare <yimizare@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/02 18:13:25 by yimizare          #+#    #+#             */
/*   Updated: 2025/07/04 20:09:44 by yimizare         ###   ########.fr       */
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
	if (tokens.size() <= i + 2 || tokens[i + 2] != "{")
		throw std::runtime_error("Syntax error: 'location' block must be followed by a path and a '{'");
	
	location.path = tokens[i + 1];
	i += 3;
	while (tokens[i] != "}" && i < tokens.size())
	{
		if (tokens[i] == "root")
		{
			if (tokens.size() <= i + 2 || tokens[i + 2] != ";")
				throw std::runtime_error("Syntax error: 'root' directive must end with a ';'");
			location.root = tokens[i + 1];
			i += 3;
		}
		else if (tokens[i] == "index")
		{
			if (tokens.size() <= i + 2 || tokens[i + 2] != ";")
				throw std::runtime_error("Syntax error: 'index' directive must end with a ';'");
			location.index = tokens[i + 1];
			i += 3;
		}
		else if (tokens[i] == "autoindex")
		{
			if (i + 2 >= tokens.size() || tokens[i + 2] != ";")
                throw std::runtime_error("Syntax error: 'autoindex' directive must end with ';'");
            location.autoindex = (tokens[i + 1] == "on");
            i += 3;
		}
		else if (tokens[i] == "allowed_methods")
		{
            i++;
            while (i < tokens.size() && tokens[i] != ";")
			{
                location.allowed_methods.push_back(tokens[i]);
                i++;
            }
            if (i == tokens.size() || tokens[i] != ";")
                throw std::runtime_error("Syntax error: 'allowed_methods' directive must end with ';'");
            i++;
        }
		else if (tokens[i] == "cgi_pass")
		{
			if (i + 2 >= tokens.size() || tokens[i + 2] != ";")
				throw std::runtime_error("Syntax error: 'cgi_pass' directive must end with ';'");
			location.cgi_pass = tokens[i + 1];
			i += 3;
		}
		else if (tokens[i] == "upload_path")
		{
			if (i + 2 >= tokens.size() || tokens[i + 2] != ";")
				throw std::runtime_error("Syntax error: 'upload_path' directive must end with ';'");
			location.upload_path = tokens[i + 1];
			i += 3;
		}
		else if (tokens[i] == "return")
		{
            if (i + 3 >= tokens.size() || tokens[i + 3] != ";")
                throw std::runtime_error("Syntax error: 'return' directive must be: return <code> <url>;");
            location.redirection_code = tokens[i + 1];
            location.redirection_url = tokens[i + 2];
            i += 4;
        }
		else
		{
			std::ostringstream oss;
			oss << "Unknown or misplaced directive in location block: '" << tokens[i] << "'";
			throw std::runtime_error(oss.str());
		}
	}
	if (tokens.size() == i)
		throw std::runtime_error("Syntax error: 'location' block not closed with '}");
	return (i + 1);
}


std::vector<ServerConfig> ConfigParser::parse(const std::vector<std::string> tokens)
{
	std::vector<ServerConfig> servers;
	size_t i = 0;
	
	while (i < tokens.size())
	{
		if (tokens[i] == "server")
		{
            size_t j = i + 1;
            while (j < tokens.size() && tokens[j] != "{") 
				++j;
            if (j == tokens.size())
                throw std::runtime_error("Syntax error: 'server' block not opened with '{'");
            ServerConfig server;
            i = j + 1;
            while (i < tokens.size() && tokens[i] != "}")
			{
				if (tokens[i] == "listen")
				{
					if (i + 2 >= tokens.size() || tokens[i + 2] != ";")
						throw std::runtime_error("Syntax error: 'listen' directive must end with ';'");
					server.listen_port = atoi(tokens[i + 1].c_str());
					i += 3;
                }
				else if (tokens[i] == "server_name")
				{
					if (i + 2 >= tokens.size() || tokens[i + 2] != ";")
						throw std::runtime_error("Syntax error: 'server_name' directive must end with a ';'");
					server.server_name = tokens[i + 1];
					i += 3;
				}
				else if (tokens[i] == "error_page")
				{
					if (i + 3 >= tokens.size() || tokens[i + 3] != ";")
						throw std::runtime_error("Syntax error: 'error_page' directive must be: error_page <code> <path>;");
					int code = atoi(tokens[i + 1].c_str());
					server.error_pages[code] = tokens[i + 2];
					i += 4;
                }
				else if (tokens[i] == "client_max_body_size")
				{
					if (i + 2 >= tokens.size() || tokens[i + 2] != ";")
						throw std::runtime_error("Syntax error: 'client_max_body_size' directive must end with ';'");
					server.client_max_body_size = atoi(tokens[i + 1].c_str());
					i += 3;
                }
				else if (tokens[i] == "location")
				{
					LocationConfig location;
					i = parse_location(tokens, i, location);
					server.locations.push_back(location);
				}
				else
				{
					std::ostringstream oss;
					oss << "Unknown or misplaced directive in server block: '" << tokens[i] << "'";
					throw std::runtime_error(oss.str());
				}
			}
			if (i == tokens.size())
				throw std::runtime_error("Syntax error: 'server' block not closed with a '}'");
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