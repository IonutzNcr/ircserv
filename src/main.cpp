#include "../includes/Server.hpp"

bool    ft_one(std::string line)
{
    for (int i = 0; i < line.size(); i++) {
        if (!std::isdigit(line[i]))
            return (false);
    }
    return (true);
}

int main(int argc, char **argv)
{
	if (argc != 3){
		std::cerr << "Usage: " << argv[0] << " <port> <password>" << std::endl;
		return 1;
	}
	if (!ft_one(argv[1])) {
		std::cout << "argv[1] is not digit" << std::endl;
		return (0);
	}
	int n = std::stoi(argv[1]);
	if (n < 1024) {		// manque la valeur la plus grande ?
		if (n < 0)
			std::cout << "argv[1] is negative" << std::endl;
		else
			std::cout << "argv[1] is less to 1024" << std::endl;
		return (0);
	}
	Server ser(std::atoi(argv[1]), argv[2]);
	std::cout << "---- SERVER ----" << std::endl;
	
	try{
		signal(SIGINT, Server::SignalHandler); //-> catch the signal (ctrl + c)
		signal(SIGQUIT, Server::SignalHandler); //-> catch the signal (ctrl + \)
		ser.ServerInit(); //-> initialize the server
	}
	catch(const std::exception& e){
		std::cerr << e.what() << std::endl;
	}
	return 0;
}