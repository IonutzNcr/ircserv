#include "server.hpp"

int main(int argc, char **argv)
{
	if (argc != 3){
		std::cerr << "Usage: " << argv[0] << " <port> <password>" << std::endl;
		return 1;
	}

	Server ser;
	std::cout << "---- SERVER ----" << std::endl;
	Authenticator *auth;
	try{
		auth = new Authenticator(std::string(argv[2]));
		signal(SIGINT, Server::SignalHandler); //-> catch the signal (ctrl + c)
		signal(SIGQUIT, Server::SignalHandler); //-> catch the signal (ctrl + \)
		ser.ServerInit(std::atoi(argv[1])); //-> initialize the server
	}
	catch(const std::exception& e){
		ser.CloseFds(); //-> close the file descriptors
		std::cerr << e.what() << std::endl;
	}
	std::cout << "The Server Closed!" << std::endl;
	delete auth;	
}