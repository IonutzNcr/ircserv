#include "../../includes/Server.hpp"
#include "../../includes/Command.hpp"
#include <errno.h>


bool Server::Signal = false; //-> initialize the static boolean

void Server::SignalHandler(int signum)
{
	(void)signum;
	std::cout << std::endl << "Signal Received!" << std::endl;
	Server::Signal = true; //-> set the static boolean to true to stop the server
}

void Server::ClearClients(int fd, Dispatch &dispatch)
{
    Client* clientToRemove = NULL;

	std::string quitMsg = "";

    for(size_t i = 0; i < clients.size(); i++){
        if (clients[i]->GetFd() == fd) {
            clientToRemove = clients[i];
            quitMsg = clientToRemove->GetMsgQuit();
            clients.erase(clients.begin() + i);
            break;
        }
    }

    if (!clientToRemove)
	{
        return;
	}
	std::string quitMsgCrtlC = ":" + clientToRemove->GetNick() + " QUIT :Client disconnected\r\n";
	if (quitMsg.empty())
		quitMsg = quitMsgCrtlC;
	for (size_t i = 0; i < dispatch._channels.size(); i++) {
		if (dispatch._channels[i]->isUserInChannel(clientToRemove)) {
			const std::vector<Client*>& channelUsers = dispatch._channels[i]->getUsers();
			for (size_t j = 0; j < channelUsers.size(); j++) {
				if (channelUsers[j]->GetFd() != fd)
					send(channelUsers[j]->GetFd(), quitMsg.c_str(), quitMsg.length(), 0);
			}
		}
	}
	
    for (size_t i = 0; i < dispatch._channels.size(); i++) {
        dispatch._channels[i]->removeUser(clientToRemove);
    }

    delete clientToRemove;
}


void Server::CloseFds(){
	for(size_t i = 0; i < clients.size(); i++){ //-> close all the clients
		std::cout << RED << "Client <" << clients[i]->GetFd() << "> Disconnected" << WHI << std::endl;
		close(clients[i]->GetFd());
	}
	if (SerSocketFd != -1){ //-> close the server socket
		std::cout << RED << "Server <" << SerSocketFd << "> Disconnected" << WHI << std::endl;
		close(SerSocketFd);
	}
}

void Server::SerSocket()
{
	struct sockaddr_in add;
	struct pollfd NewPoll;
	add.sin_family = AF_INET; //-> set the address family to ipv4
	add.sin_port = htons(this->Port); //-> convert the port to network byte order (big endian)
	add.sin_addr.s_addr = INADDR_ANY; //-> set the address to any local machine address

	SerSocketFd = socket(AF_INET, SOCK_STREAM, 0); //-> create the server socket
	if(SerSocketFd == -1) //-> check if the socket is created
		throw(std::runtime_error("faild to create socket"));

	int en = 1;
	if(setsockopt(SerSocketFd, SOL_SOCKET, SO_REUSEADDR, &en, sizeof(en)) == -1) //-> set the socket option (SO_REUSEADDR) to reuse the address
		throw(std::runtime_error("faild to set option (SO_REUSEADDR) on socket"));
	if (fcntl(SerSocketFd, F_SETFL, O_NONBLOCK) == -1) //-> set the socket option (O_NONBLOCK) for non-blocking socket
		throw(std::runtime_error("faild to set option (O_NONBLOCK) on socket"));
	if (bind(SerSocketFd, (struct sockaddr *)&add, sizeof(add)) == -1) //-> bind the socket to the address
		throw(std::runtime_error("faild to bind socket"));
	if (listen(SerSocketFd, SOMAXCONN) == -1) //-> listen for incoming connections and making the socket a passive socket
		throw(std::runtime_error("listen() faild"));

	NewPoll.fd = SerSocketFd; //-> add the server socket to the pollfd
	NewPoll.events = POLLIN; //-> set the event to POLLIN for reading data
	NewPoll.revents = 0; //-> set the revents to 0
	fds.push_back(NewPoll); //-> add the server socket to the pollfd
}

void Server::ServerInit()
{
	SerSocket(); //-> create the server socket

	std::cout << GRE << "Server <" << SerSocketFd << "> Connected" << WHI << std::endl;
	std::cout << "Waiting to accept a connection...\n";
	Dispatch dispatch(_password, clients);
    while (Server::Signal == false) //-> run the server until the signal is received
	{
		if((poll(&fds[0],fds.size(),-1) == -1) && Server::Signal == false) //-> wait for an event
			throw(std::runtime_error("poll() faild"));

		for (size_t i = 0; i < fds.size(); i++)
		{
			if (fds[i].revents & (POLLHUP | POLLERR | POLLNVAL))
			{
				int deadFd = fds[i].fd;
				std::cout << "FD " << deadFd << " closed (poll error)" << std::endl;
				close(deadFd);
				ClearClients(deadFd, dispatch);
				fds.erase(fds.begin() + i);
				i--;
				continue;
			}

			if (fds[i].revents & POLLIN)
			{
				if (fds[i].fd == SerSocketFd)
					AcceptNewClient();
				else
					ReceiveNewData(fds[i].fd, dispatch);
			}
		}
        
		std::cout << "loop server\n";

	}
}



void Server::AcceptNewClient()
{
	struct sockaddr_in cliadd;
	struct pollfd NewPoll;
	socklen_t len = sizeof(cliadd);

	int incofd = accept(SerSocketFd, (sockaddr *)&(cliadd), &len); //-> accept the new client
	if (incofd == -1)
		{std::cout << "accept() failed" << std::endl; return;}

	//Find another way to set non-blocking
	if (fcntl(incofd, F_SETFL, O_NONBLOCK) == -1) //-> set the socket option (O_NONBLOCK) for non-blocking socket
		{std::cout << "fcntl() failed" << std::endl; return;}
	NewPoll.fd = incofd; //-> add the client socket to the pollfd
	NewPoll.events = POLLIN; //-> set the event to POLLIN for reading data
	NewPoll.revents = 0; //-> set the revents to 0
	Client *cli = new Client(incofd);
	cli->SetIpAdd(inet_ntoa((cliadd.sin_addr))); //-> convert the ip address to string and set it
	clients.push_back(cli); //-> add the client to the vector of clients
	fds.push_back(NewPoll); //-> add the client socket to the pollfd
	//send(incofd, "Welcome to the server!\n", 23, 0); //-> send a welcome message to the client
	std::cout << GRE << "Client <" << incofd << "> Connected" << WHI << std::endl;
}

void Server::ReceiveNewData(int fd, Dispatch &dispatch)
{
	char buff[1024]; //-> buffer for the received data
	memset(buff, 0, sizeof(buff)); //-> clear the buffer

	ssize_t bytes = recv(fd, buff, sizeof(buff) - 1 , 0); //-> receive the data
	
	if (bytes == 0)
	{
		ClearClients(fd, dispatch);
		close(fd);

		for (size_t i = 0; i < fds.size(); i++)
		{
			if (fds[i].fd == fd)
			{
				fds.erase(fds.begin() + i);
				break;
			}
		}
		return;
	}
	else if (bytes == -1)
	{
		if (errno != EWOULDBLOCK && errno != EAGAIN) //-> check if the error is not EWOULDBLOCK or EAGAIN
			std::cout << "recv() failed" << std::endl;
	}
	else{ //-> print the received data
		buff[bytes] = '\0';
		Command cmd;
		parse.fill(buff, fd);
		buff[0] = '\0';
		std::cout << "In receive new data, about to get command " << parse.getCmdtwo(fd) << std::endl;
		std::string line = parse.getCmdtwo(fd);
		cmd = parse.get(fd);
		cmd.setLine(line);
		while (!cmd.getCmd().empty())
		{
			std::cout << "Dispatching command: " << cmd.getLine() << std::endl;
			dispatch.dispatch(cmd, fd);
			line = parse.getCmdtwo(fd);
			cmd = parse.get(fd);
			cmd.setLine(line);
		}
		//std::cout << YEL << "Client <" << fd << "> Data: " << WHI << buff;
		//here you can add your code to process the received data: parse, check, authenticate, handle the command, etc...
	}
}

int	Server::findClient(std::string nick)
{
	for(size_t i = 0; i < clients.size(); i++){
		if (clients[i]->GetNick() == nick) {
                return (clients[i]->GetFd());
            }
	}
	return (-1);
}