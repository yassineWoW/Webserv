#include "multiplexer.hpp"
#include "cfileparser.hpp"

int main()
{
	try
	{
		ConfigParser *parser = ConfigParser::getInstance("webserv.conf");
		std::vector<ServerConfig> &servers = parser->getServers();
		for (size_t i = 0; i < servers.size(); ++i) 
		{
			std::cout << "Server " << i << ":\n";
			std::cout << "  Port: " << servers[i].listen_port << "\n";
			std::cout << "  Server Name: " << servers[i].server_name << "\n";
			for (size_t j = 0; j < servers[i].locations.size(); ++j) {
				std::cout << "    Location " << servers[i].locations[j].path << ":\n";
				std::cout << "      Root: " << servers[i].locations[j].root << "\n";
				std::cout << "      Index: " << servers[i].locations[j].index << "\n";
			}
		}
	}
	catch (const std::exception &e)
	{
		std::cerr << "Config file parsing error: " << e.what() << std::endl;
		return 1;
	}


	return 0;
}