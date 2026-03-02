
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
    for (size_t i = 0; i < _channels.size(); i++)
        delete _channels[i];
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
        ft_privmsg(cmd, fd);
    else if (cmd.getCmd() == "PING")
        ft_ping(cmd, fd);
    else if (cmd.getCmd() == "QUIT")
        ft_quit(cmd, fd);
    else if (cmd.getCmd() == "PART")
        ft_part(cmd, fd);
    return true;
}

bool Dispatch::ft_cap(Command cmd, int fd)
{
    std::string line = cmd.getLine();
    if (line.find("LS") != std::string::npos) {
        std::string msg = "CAP * LS :\r\n";
        sendAll(fd, msg);
    }
    else if (line.find("REQ") != std::string::npos) {
        std::string msg = "CAP * NAK :\r\n";
        sendAll(fd, msg);
    }
    else if (line.find("END") != std::string::npos) {
       ;
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
        sendAll(client->GetFd(), msg);
    }
    if (!client->isAuthenticated() && !client->GetNick().empty() && !client->GetUser().empty()) {
        std::string msg2 = ":server 464 * :Password incorrect\r\n";
        sendAll(client->GetFd(), msg2);
        // marquer le fd pour suppression  Server::removeFdPoll() va le nettoyer
        client->SetFd(-1);
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
        token = line.substr(5);
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
    sendAll(fd, reply);
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
    delete channel;
    return ;
}