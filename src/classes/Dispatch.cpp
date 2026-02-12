
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

//TODO::RPL_TOPIC
//TODO::RPL_NAMREPLY
bool Dispatch::ft_join(Command cmd, int fd)
{
    Client* client = getClientFd(fd);
    if (!client)
        return false;
     if (!client->isRegistered()) // si le client n'es pas register just return false
        return false;
    std::string line = cmd.getLine();
    std::string channelName = line.substr(5); // on stock la string apres le JOIN
    std::vector<std::string> chanXkeys = split(channelName, ' ');
    if (chanXkeys.empty())
    {
        std::string msg = ":server 461 JOIN :Not enough parameters\r\n";
        send(fd, msg.c_str(), msg.length(), 0);
        return false;
    }
    std::string chanNames = chanXkeys[0];
    bool isKey = false;
    if (chanXkeys.size() > 1)
        isKey = true;
    std::string key;
    if (isKey)
        key = chanXkeys[1];
    std::vector<std::string> chanNamesSplit = split(chanNames, ',');
    std::vector<std::string>keysSplit;
    if (isKey)
        keysSplit = split(key, ',');
    for (size_t i = 0; i < chanNamesSplit.size(); i++)
    {
        std::string chanName = chanNamesSplit[i];
        std::string chanKey;
        if (isKey && i < keysSplit.size())
            chanKey = keysSplit[i];
        else
            chanKey = "";
        //creation channel si n'existe pas et ajout user au channel
        Channel *newChan = new Channel("", chanName, i, chanKey); // to fix the id is not really unique
        if (!isChannelExist(chanName)) // TODO:: big problem why ?
        {
            //print les channels existants
            std::cout << "Existing channels: ";
            for (size_t j = 0; j < _channels.size(); j++)
            {
                std::cout << _channels[j]->getName() << " ";
            }
            std::cout << std::endl;
            
            _channels.push_back(newChan);
            newChan->addUser(client);
            newChan->addOperator(client);
            std::string msg = ":" + client->GetNick() + " JOIN " + chanName + "\r\n";
            send(fd, msg.c_str(), msg.length(), 0);
            //RPL_NOTOPIC
            msg = ":server 331 " + client->GetNick() + " " + newChan->getName() + " :No topic is set\r\n";
            send(fd, msg.c_str(), msg.size(), 0);
            //RPL_NAMREPLY
            msg = ":server 353 " + client->GetNick() + " = " + newChan->getName() + " :@" + client->GetNick() + "\r\n";
            send(fd, msg.c_str(), msg.size(), 0);
            //RPL_ENDOFNAMES
            msg = ":server 366 " + client->GetNick() + " " + newChan->getName() + " :End of /NAMES list\r\n";
            send(fd, msg.c_str(), msg.size(), 0);
        }else
        {
            for (size_t j = 0; j < _channels.size(); j++)
            {
                if (_channels[j]->getName() == chanName)
                {
                    std::string  msg;
                    if (_channels[j]->isUserInChannel(client))
                    {
                       /*  msg = "User is already in the channel " + chanName + "\r\n";
                        send(fd, msg.c_str(), msg.length(), 0); */
                        continue; // a voir que faire si deja dans le channel et mauvais key
                    }
                    if (_channels[j]->getKey() != chanKey)
                    {
                        std::string errMsg = ":server 475 " + client->GetNick() + " " + chanName + " :Cannot join channel (+k)\r\n";
                        send(fd, errMsg.c_str(), errMsg.length(), 0);
                        continue;
                    }

                    // Broadcast JOIN to all existing members
                    std::string joinMsg = ":" + client->GetNick() + " JOIN " + chanName + "\r\n";
                    std::vector<Client *> existingUsers = _channels[j]->getUsers();
                    for (std::size_t k = 0; k < existingUsers.size(); k++)
                    {
                        send(existingUsers[k]->GetFd(), joinMsg.c_str(), joinMsg.length(), 0);
                    }
                    // Also send JOIN confirmation to the joining client
                    send(fd, joinMsg.c_str(), joinMsg.length(), 0);
                    _channels[j]->addUser(client);
                    if(_channels[j]->getTopic().empty())
                    {
                        msg = ":server 331 " + client->GetNick() + " " + _channels[j]->getName() + " :No topic is set\r\n";
                        send(fd, msg.c_str(), msg.size(), 0);
                    } else
                    {
                        msg = ":server 332 " + client->GetNick() + " " + _channels[j]->getName() + " :" + _channels[j]->getTopic() + "\r\n"; 
                        send(fd, msg.c_str(), msg.size(), 0);
                    }
                    //RPL_NAMREPLY - list all users with @ prefix for operators
                    msg = ":server 353 " + client->GetNick() + " = " + _channels[j]->getName() + " :";
                    std::vector<Client *> channelUsers = _channels[j]->getUsers();
                    for (std::size_t k = 0; k < channelUsers.size(); k++)
                    {
                        if (_channels[j]->isOperator(channelUsers[k]))
                            msg += "@";
                        msg += channelUsers[k]->GetNick();
                        if (k < channelUsers.size() - 1)
                            msg += " ";
                    }
                    msg += "\r\n";
                    send(fd, msg.c_str(), msg.size(), 0);
                    //RPL_ENDOFNAMES
                    msg = ":server 366 " + client->GetNick() + " " + _channels[j]->getName() + " :End of /NAMES list\r\n";
                    send(fd, msg.c_str(), msg.size(), 0);
                }
            }
        }        
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