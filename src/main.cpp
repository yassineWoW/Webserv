 #include "multiplexer.hpp"
 #include "cfileparser.hpp"

volatile sig_atomic_t stop_server = 0;

void hanle_stop_signal(int signum)
{
	(void)signum;
	stop_server = 1;
	std::cout << "\n\033[31mSIGINT called!!\033[0m" << std::endl;
}

 int main(int ac, char **av)
 {
 	std::string config_file;
 	if (ac == 2)
 	{
 		config_file = (av[1]);
 	}
 	else
 	{
 		config_file = ("webserv.conf");
 	}
	signal(SIGPIPE, SIG_IGN);
	signal(SIGINT, hanle_stop_signal);
	signal(SIGTERM, hanle_stop_signal);
	
	try
	{
		ConfigParser *parser = ConfigParser::getInstance(config_file);
		 std::vector<ServerConfig> &servers = parser->getServers();
		Multiplexer *server = Multiplexer::getInstance();
		for (size_t j = 0; j < servers.size(); ++j)
		{
			server->server_ports.push_back(servers[j].listen_port);
		}
		// for (size_t i = 0; i < servers.size(); ++i) 
		// {
		// 	std::cout << "Server " << i << ":\n";
		// 	std::cout << "  Port: " << servers[i].listen_port << "\n";
		// 	std::cout << "  Server Name: " << servers[i].server_name << "\n";
		// 	for (size_t j = 0; j < servers[i].locations.size(); ++j) {
		// 		std::cout << "    Location " << servers[i].locations[j].path << ":\n";
		// 		std::cout << "      Root: " << servers[i].locations[j].root << "\n";
		// 		std::cout << "      Index: " << servers[i].locations[j].index << "\n";
		// 	}
		// }
        //server();
		
	std::cout << "\033[33mWebserv listening on port " << server->server_ports[0] << "...\033[0m" << std::endl;		
	server->run();
	}
	catch (const std::exception &e)
	{
		std::cerr << "Fatal error: " << e.what() << std::endl;
		return 1;
	}


 	return 0;
 }