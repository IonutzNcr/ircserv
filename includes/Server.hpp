#pragma once

#include <vector> 
#include <sys/socket.h> 
#include <sys/types.h> 
#include <netinet/in.h> 
#include <fcntl.h> 
#include <unistd.h> 
#include <arpa/inet.h> 
#include <poll.h> 
#include <csignal>
#include <iostream>
#include <cstring> 
#include <cstdlib>
#include <stdexcept>
#include <string>

#include "Client.hpp"
#include "Parser.hpp"
#include "Dispatch.hpp"

#define RED "\e[1;31m" 
#define WHI "\e[0;37m" 
#define GRE "\e[1;32m" 
#define YEL "\e[1;33m"


class Server
{
	private:
		int Port;
		int SerSocketFd;
		static bool Signal;
		std::string _password;
		Parser parse;
		std::vector<Client *> clients;
		std::vector<struct pollfd> fds; 
		void removeFdPoll();
	public:
		Server() {return ;}
		Server(int port, std::string password):Port(port), _password(password) {
			SerSocketFd = -1;}
		~Server() 
		{
			CloseFds();
			for (size_t i = 0; i < clients.size(); i++)
				delete clients[i];

			std::cout << "The Server Closed!" << std::endl;
		};
		void ServerInit();
		void SerSocket(); 
		void AcceptNewClient();
		void ReceiveNewData(int fd, Dispatch &dispatch);
		int	findClient(std::string nick);

		static void SignalHandler(int signum); 
	
		void CloseFds();
		void ClearClients(int fd, Dispatch &dispatch);
};


