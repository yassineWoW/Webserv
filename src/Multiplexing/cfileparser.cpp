/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cfileparser.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yimizare <yimizare@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/02 18:13:25 by yimizare          #+#    #+#             */
/*   Updated: 2025/07/17 17:41:04 by yimizare         ###   ########.fr       */
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
	if (location.path.empty() || location.path[0] != '/')
        throw std::runtime_error("Invalid location path: must start with '/' and not be empty");
	i += 3;

	bool root_found = false, index_found = false, autoindex_found = false, cgi_pass_found = false, upload_path_found = false, return_found = false;
    bool allowed_methods_found = false;
	std::set<std::string> allowed_methods_set;
	while (tokens[i] != "}" && i < tokens.size())
	{
		if (tokens[i] == "location" || tokens[i] == "server")
		{
        	throw std::runtime_error("Syntax error: 'location' block not closed with '}' before starting new block");
    	}
		if (tokens[i] == "root")
		{
			if (root_found)
                throw std::runtime_error("Duplicate 'root' directive in location block: " + location.path);
            root_found = true;
			if (tokens.size() <= i + 2 || tokens[i + 2] != ";")
				throw std::runtime_error("Syntax error: 'root' directive must end with a ';'");
			location.root = tokens[i + 1];
			if (location.root.empty() || location.root[0] != '/')
                throw std::runtime_error("Invalid root path in location block: " + location.path);
			i += 3;
		}
		else if (tokens[i] == "index")
		{
			if (index_found)
                throw std::runtime_error("Duplicate 'index' directive in location block: " + location.path);
            index_found = true;
			if (tokens.size() <= i + 2 || tokens[i + 2] != ";")
				throw std::runtime_error("Syntax error: 'index' directive must end with a ';'");
			location.index = tokens[i + 1];
			if (location.index.empty())
                throw std::runtime_error("index cannot be empty in location block: " + location.path);
			i += 3;
		}
		else if (tokens[i] == "autoindex")
		{
			if (autoindex_found)
                throw std::runtime_error("Duplicate 'autoindex' directive in location block: " + location.path);
            autoindex_found = true;
            if (i + 2 >= tokens.size() || tokens[i + 2] != ";")
                throw std::runtime_error("Syntax error: 'autoindex' directive must end with ';'");
            if (tokens[i + 1] != "on" && tokens[i + 1] != "off")
                throw std::runtime_error("autoindex must be 'on' or 'off' in location block: " + location.path);
            location.autoindex = (tokens[i + 1] == "on");
            i += 3;
		}
		else if (tokens[i] == "allowed_methods")
		{
            if (allowed_methods_found)
                throw std::runtime_error("Duplicate 'allowed_methods' directive in location block: " + location.path);
            allowed_methods_found = true;
            std::set<std::string> allowed;
            i++;
            while (i < tokens.size() && tokens[i] != ";") {
                std::string method = tokens[i];
                if (method != "GET" && method != "POST" && method != "DELETE")
                    throw std::runtime_error("Invalid HTTP method in allowed_methods: " + method);
                if (!allowed.insert(method).second)
                    throw std::runtime_error("Duplicate HTTP method in allowed_methods: " + method);
                location.allowed_methods.push_back(method);
                i++;
            }
            if (i == tokens.size() || tokens[i] != ";")
                throw std::runtime_error("Syntax error: 'allowed_methods' directive must end with ';'");
            i++;
        }
		else if (tokens[i] == "cgi_pass")
		{
            if (cgi_pass_found)
                throw std::runtime_error("Duplicate 'cgi_pass' directive in location block: " + location.path);
            cgi_pass_found = true;
            if (i + 2 >= tokens.size() || tokens[i + 2] != ";")
                throw std::runtime_error("Syntax error: 'cgi_pass' directive must end with ';'");
            location.cgi_pass = tokens[i + 1];
            if (location.cgi_pass.empty() || location.cgi_pass[0] != '/')
                throw std::runtime_error("Invalid cgi_pass path in location block: " + location.path);
            i += 3;
        }
		else if (tokens[i] == "upload_path")
		{
            if (upload_path_found)
                throw std::runtime_error("Duplicate 'upload_path' directive in location block: " + location.path);
            upload_path_found = true;
			if (tokens[i + 1].empty() || tokens[i + 1] == ";")
			    throw std::runtime_error("upload_path value cannot be empty in location block: " + location.path);
			if (i + 2 >= tokens.size() || tokens[i + 2] != ";")
    			throw std::runtime_error("Syntax error: 'upload_path' directive must end with ';'");
			location.upload_path = tokens[i + 1];
			if (location.upload_path[0] != '/')
			    throw std::runtime_error("Invalid upload_path in location block: " + location.path);
			i += 3;
        }
		else if (tokens[i] == "return")
		{
            if (return_found)
                throw std::runtime_error("Duplicate 'return' directive in location block: " + location.path);
            return_found = true;
            if (i + 3 >= tokens.size() || tokens[i + 3] != ";")
                throw std::runtime_error("Syntax error: 'return' directive must be: return <code> <url>;");
            // Strict numeric check for code
            std::string code_str = tokens[i + 1];
            int code = 0;
            std::stringstream ss(code_str);
            ss >> code;
            if (ss.fail() || !ss.eof() || code < 300 || code > 399)
                throw std::runtime_error("Invalid return code in location block: " + code_str);
            location.redirection_code = tokens[i + 1];
            location.redirection_url = tokens[i + 2];
            if (location.redirection_url.empty())
                throw std::runtime_error("Redirection URL cannot be empty in location block: " + location.path);
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
		throw std::runtime_error("Syntax error: 'location' block not closed with }'");
	return (i + 1);
}


void	ConfigParser::parse_port(ServerConfig &server, size_t i, std::vector<std::string> tokens)
{
	if (i + 2 >= tokens.size() || tokens[i + 2] != ";")
		throw std::runtime_error("Syntax error: 'listen' directive must end with ';'");
	server.listen_port = atoi(tokens[i + 1].c_str());
	if (server.listen_port < 1 || server.listen_port > 65535)
		throw std::runtime_error("Invalid Port value");
}

void ConfigParser::parse_name(ServerConfig &server, size_t i, std::vector<std::string> tokens)
{
	if (i + 2 >= tokens.size() || tokens[i + 2] != ";")
		throw std::runtime_error("Syntax error: 'server_name' directive must end with a ';'");
	std::string name = tokens[i + 1];
	if (name.empty())
		throw std::runtime_error("server_name cannot be empty!");
	for (size_t c = 0; c < name.size(); ++c)
	{
		char ch = name[c];
		if (!std::isalnum(ch) && ch != '.' && ch != '-')
			throw std::runtime_error("server_name contains invalid characters!" + name);
	}
	server.server_name = tokens[i + 1];
}

void ConfigParser::parse_error_code(ServerConfig &server, size_t i, std::vector<std::string> tokens)
{
	if (i + 3 >= tokens.size() || tokens[i + 3] != ";")
		throw std::runtime_error("Syntax error: 'error_page' directive must be: error_page <code> <path>;");
	int code = atoi(tokens[i + 1].c_str());
	std::string path = tokens[i + 1];
	if (code < 400 || code > 599)
		throw std::runtime_error("error_page code must be between 400 and 599");
	if (path.empty())
		throw std::runtime_error("error_page path cannot be empty!");
	server.error_pages[code] = tokens[i + 2];
}

void ConfigParser::parse_max_body_size(ServerConfig &server, size_t i, std::vector<std::string> tokens)
{
	if (i + 2 >= tokens.size() || tokens[i + 2] != ";")
		throw std::runtime_error("Syntax error: 'client_max_body_size' directive must end with ';'");
	std::string value = tokens[i + 1];
	size_t size = 0;
	if (value.size() >= 2 && isdigit(value[0]))
	{
		size_t num_end = value.find_first_not_of("0123456789");
		std::string num_str = value.substr(0, num_end);
		std::string suffix = value.substr(num_end);
		std::stringstream ss(num_str);
		ss >> size;
		if (ss.fail() || size == 0)
			throw std::runtime_error("client_max_body_size must be a positive integer");
		if (suffix == "KB")
			size *= 1024;
		else if (suffix == "MB")
			size *= 1024 * 1024;
		else if (suffix == "GB")
			size *= 1024 * 1024 * 1024;
		else if (!suffix.empty())
			throw std::runtime_error("Unknown size suffix for client_max_body_size");
		if (size > 2 * 1024 * 1024)
			throw std::runtime_error("client_max_body_size is too large (max is 2MB)");
	}
	else
		throw std::runtime_error("Invalid client_max_body_size value");
	server.client_max_body_size = size;
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
            bool listen_found = false;
			bool server_name_found = false;
			bool cmbs = false;
			std::set<int> error_codes;
			std::set<std::string> location_paths;
			while (i < tokens.size() && tokens[i] != "}")
			{
				if (tokens[i] == "listen")
				{
					if (listen_found)
            			throw std::runtime_error("Duplicate 'listen' directive in the same server block");
        			listen_found = true;
					parse_port(server, i, tokens);
					i += 3;
                }
				else if (tokens[i] == "server_name")
				{
					if (server_name_found)
						throw std::runtime_error("Duplicate 'server_name' directive in the same server block");
					server_name_found = true;
					parse_name(server, i,tokens);
					i += 3;
				}
				else if (tokens[i] == "error_page")
				{
					int code = atoi(tokens[i + 1].c_str());
					if (!error_codes.insert(code).second)
						throw std::runtime_error("Duplicate error_page code in the same server block: " + tokens[i + 1]);
					parse_error_code(server ,i , tokens);
					i += 4;
                }
				else if (tokens[i] == "client_max_body_size")
				{
					if (cmbs)
						throw std::runtime_error("Duplicate client max body size in the same server block");
					cmbs = true;
					parse_max_body_size(server, i, tokens);
					i += 3;
                }
				else if (tokens[i] == "location")
				{
					std::string path = tokens[i + 1];
					if (!location_paths.insert(path).second)
						throw std::runtime_error("Duplicate location path in the same server block: " + path);
					LocationConfig location;
					size_t new_i = parse_location(tokens, i, location);
					server.locations.push_back(location);
					i = new_i;
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
			for (size_t s = 0; s < servers.size(); ++s)
			{
			    if (servers[s].listen_port == server.listen_port) {
			        std::ostringstream oss;
			        oss << "Duplicate listen port in config: " << server.listen_port;
			        throw std::runtime_error(oss.str());
			    }
			    if (servers[s].server_name == server.server_name) {
			        std::ostringstream oss;
			        oss << "Duplicate server_name in config: " << server.server_name;
			        throw std::runtime_error(oss.str());
			    }
			}
			std::set<int> error_codes1;
			for (std::map<int, std::string>::const_iterator it = server.error_pages.begin(); it != server.error_pages.end(); ++it)
			{
			    if (!error_codes1.insert(it->first).second)
			    {
			        std::ostringstream oss;
			        oss << "Duplicate error_page code in config: " << it->first;
			        throw std::runtime_error(oss.str());
			    }
			}
			std::set<std::string> location_paths1;
			for (size_t l = 0; l < server.locations.size(); ++l)
			{
			    if (!location_paths1.insert(server.locations[l].path).second)
			        throw std::runtime_error("Duplicate location path in server: " + server.locations[l].path);
			}
			servers.push_back(server);
		}
		++i;
	}
	return (servers);
}

void ConfigParser::destroyInstance()
{
	if (instance)
	{
		delete instance;
		instance = NULL;
	}
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