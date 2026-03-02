
#include "../../includes/Dispatch.hpp"
#include "../../includes/Command.hpp"
#include "../../includes/Client.hpp"
#include "../../includes/split.hpp"
#include "../../includes/Channel.hpp"
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>


Dispatch::Dispatch(std::string pass, std::vector<Client *>&clients): _password(pass), _clients(clients)
{
    return ;
}

Dispatch::~Dispatch()
{
    return ;
}

bool Dispatch::dispatch(Command cmd, int fd)
{
    if (cmd.getCmd() == "CAP")
        ft_cap(cmd, fd);
    else if (cmd.getCmd() == "PASS")
    {
        if (!ft_pass(cmd, fd))
            return false;
    }
    else if (cmd.getCmd() == "NICK")
    {
        if (!ft_nick(cmd, fd))
            return false;
    }
    else if (cmd.getCmd() == "USER")
        ft_user(cmd, fd);
    else if (cmd.getCmd() == "JOIN")
        ft_join(cmd, fd);
    else if (cmd.getCmd() == "MODE")
        ft_mode(cmd, fd);
    else if (cmd.getCmd() == "KICK")
        ft_kick(cmd, fd);
    else if (cmd.getCmd() == "INVITE")
        ft_invite(cmd, fd);
    else if (cmd.getCmd() == "TOPIC")
        ft_topic(cmd, fd);
    else if (cmd.getCmd() == "PRIVMSG")
        ft_PRIVMSG(cmd, fd);
    else if (cmd.getCmd() == "PING")
        ft_ping(cmd, fd);
    else if (cmd.getCmd() == "QUIT")
        ft_quit(cmd, fd);
    else if (cmd.getCmd() == "WHO")
        ft_who(cmd, fd);
    return true;
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
        std::string msg = ":server 001 " + client->GetNick() + " :Welcome to the IRC Network, " + client->GetNick() + "\r\n";
        send(client->GetFd(), msg.c_str(), msg.length(), 0);
    }
    if (!client->isAuthenticated() && !client->GetNick().empty() && !client->GetUser().empty()) {
        std::string msg2 = "464 * :Password incorrect\r\n";
        send(client->GetFd(), msg2.c_str(), msg2.length(), 0);
        for (size_t i = 0; i < _clients.size(); ++i) {
            if (_clients[i]->GetFd() == client->GetFd()) {
                close(_clients[i]->GetFd());
                _clients.erase(_clients.begin() + i);
                break;
            }
        }
        delete client;
    }
}

bool Dispatch::isChannelExist(std::string chanName)
{
    for (size_t i = 0; i < _channels.size(); i++)
    {
        if (ircCaseEqual(_channels[i]->getName(), chanName))
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
    
    // Remove leading colon if present
    if (!token.empty() && token[0] == ':')
        token = token.substr(1);
    
    std::string reply = ":server PONG server :" + token + "\r\n";
    send(fd, reply.c_str(), reply.length(), 0);
    return true;
}

Channel*    Dispatch::getChannel(std::string target)
{
    for (size_t i = 0; i < _channels.size(); i++) {
        if (ircCaseEqual(_channels[i]->getName(), target))
            return (_channels[i]);
    }
    return (NULL);
}

int	Dispatch::findClient(std::string nick)
{
	for(size_t i = 0; i < _clients.size(); i++){
		if (ircCaseEqual(_clients[i]->GetNick(), nick)) {
                return (_clients[i]->GetFd());
            }
	}
	return (-1);
}

void Dispatch::removeChannel(Channel *channel)
{
    if (!channel)
        return ;
    for (size_t i = 0; i < _channels.size(); i++)
    {
        if (channel == _channels[i])
            _channels.erase(_channels.begin() + i);
    }
    return ;
}