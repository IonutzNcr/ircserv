#pragma once

#include <vector> //-> for vector
#include <sys/socket.h> //-> for socket()
#include <sys/types.h> //-> for socket()
#include <netinet/in.h> //-> for sockaddr_in
#include <fcntl.h> //-> for fcntl()
#include <unistd.h> //-> for close()
#include <arpa/inet.h> //-> for inet_ntoa()
#include <poll.h> //-> for poll()
#include <csignal> //-> for signal()
#include <bits/stdc++.h>
#include <stdexcept>
#include <string>

#include "Client.hpp"
#include "Parser.hpp"
#include "Dispatch.hpp"

#define RED "\e[1;31m" //-> for red color
#define WHI "\e[0;37m" //-> for white color
#define GRE "\e[1;32m" //-> for green color
#define YEL "\e[1;33m" //-> for yellow color


class Server //-> class for server
{
private:
	int Port; //-> server port
	int SerSocketFd; //-> server socket file descriptor
	static bool Signal;
	std::string _password;
	Parser parse;
	/* Dispatch dispatch; */
	std::vector<Client *> clients;
	std::vector<struct pollfd> fds; 
public:
	Server(int port, std::string password):Port(port), _password(password) {
		/* dispatch = dispatch() */
		SerSocketFd = -1;}
	~Server() 
	{
		CloseFds();
    	for (size_t i = 0; i < clients.size(); i++)
   	 	delete clients[i];

		std::cout << "The Server Closed!" << std::endl;
	};
	void ServerInit();
	void SerSocket(); //-> server socket creation
	void AcceptNewClient(); //-> accept new client
	void ReceiveNewData(int fd, Dispatch &dispatch); //-> receive new data from a registered client
	int	findClient(std::string nick);

	static void SignalHandler(int signum); //-> signal handler
 
	void CloseFds(); //-> close file descriptors
	void ClearClients(int fd, Dispatch &dispatch); //-> clear clients
};


