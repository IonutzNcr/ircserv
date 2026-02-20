
#include "../../includes/Dispatch.hpp"
#include "../../includes/Command.hpp"
#include "../../includes/Client.hpp"
#include "../../includes/split.hpp"
#include "../../includes/Channel.hpp"
#include <string>
#include <sys/types.h>
#include <sys/socket.h>


Dispatch::Dispatch(std::string pass, std::vector<Client *>&clients): _password(pass), _clients(clients)
{
    return ;
}

Dispatch::~Dispatch()
{
    return ;
}

void Dispatch::dispatch(Command cmd, int fd)
{
    int choice = 0;

    if (cmd.getCmd() == "CAP")
        ft_cap(cmd, fd);
    if (cmd.getCmd() == "PASS")
        ft_pass(cmd, fd);
    if (cmd.getCmd() == "NICK")
        ft_nick(cmd, fd);
    if (cmd.getCmd() == "USER")
        ft_user(cmd, fd);
    if (cmd.getCmd() == "JOIN")
        ft_join(cmd, fd);
    if (cmd.getCmd() == "MODE")
        ft_mode(cmd, fd);
    if (cmd.getCmd() == "KICK")
        ft_kick(cmd, fd);
    if (cmd.getCmd() == "INVITE")
        ft_invite(cmd, fd);
    if (cmd.getCmd() == "TOPIC")
        ft_topic(cmd, fd);
    if (cmd.getCmd() == "PRIVMSG")
        ft_PRIVMSG(cmd, fd);
    if (cmd.getCmd() == "PING")
        ft_ping(cmd, fd);
    if (cmd.getCmd() == "QUIT")
        ft_quit(cmd, fd);
    if (cmd.getCmd() == "WHO")
        ft_who(cmd, fd);

}

bool Dispatch::ft_cap(Command cmd, int fd)
{
    std::string line = cmd.getLine();
    // respond minimally so clients finish capability negotiation
    if (line.find("LS") != std::string::npos) {
        std::string msg = "CAP * LS :\r\n";
        send(fd, msg.c_str(), msg.length(), 0);
    }
    else if (line.find("REQ") != std::string::npos) {
        std::string msg = "CAP * NAK :\r\n";
        send(fd, msg.c_str(), msg.length(), 0);
    }
    else if (line.find("END") != std::string::npos) {
        // nothing needed, client ends negotiation
    }
    return true;
}

Client *Dispatch::getClientFd(int fd_client)
{
    for (size_t i = 0; i < _clients.size(); ++i) {
            if (_clients[i]->GetFd() == fd_client)
                return (_clients[i]);
        }
    return (NULL);
}

void    Dispatch::tryRegister(Client* client)
{
    if (client->isRegistered())
        return ;
    if (client->isAuthenticated() && !client->GetNick().empty() && !client->GetUser().empty()) {
        client->setRegistered(true);
        std::string msg = ":server 001 " + client->GetNick() + " :Welcome to the IRC server!\r\n";
        send(client->GetFd(), msg.c_str(), msg.length(), 0);
    }
    if (!client->isAuthenticated() && !client->GetNick().empty() && !client->GetUser().empty()) {
        std::string msg2 = "464 * :Password incorrect\r\n";
        send(client->GetFd(), msg2.c_str(), msg2.length(), 0);
        // a voir si on deconnecte le client ou pas
    }
}

bool Dispatch::isChannelExist(std::string chanName)
{
    for (size_t i = 0; i < _channels.size(); i++)
    {
        if (_channels[i]->getName() == chanName)
            return true;
    }
    return false;
}

bool Dispatch::ft_ping(Command cmd, int fd)
{
    std::string line = cmd.getLine();
    std::string token;
    if (line.size() > 5)
        token = line.substr(5);  // après "PING "
    else
        token = "server";
    
    // Trim whitespace
    size_t start = token.find_first_not_of(" \t\r\n");
    size_t end = token.find_last_not_of(" \t\r\n");
    if (start != std::string::npos && end != std::string::npos)
        token = token.substr(start, end - start + 1);
    
    std::string reply = ":server PONG server :" + token + "\r\n";
    send(fd, reply.c_str(), reply.length(), 0);
    return true;
}

Channel*    Dispatch::getChannel(std::string target)
{
    for (size_t i = 0; i < _channels.size(); i++) {
        if (_channels[i]->getName() == target)
            return (_channels[i]);
    }
    return (NULL);
}

int	Dispatch::findClient(std::string nick)
{
	for(size_t i = 0; i < _clients.size(); i++){
		if (_clients[i]->GetNick() == nick) {
                return (_clients[i]->GetFd());
            }
	}
	return (-1);
}